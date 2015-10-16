import sys
import socket
import select
import threading
import multiprocessing
import time
from QueueManager import QueueManager
from msgHandler import msgHandler
from SVStaticData import SVStaticData
import md5


def processWorker():
    md = Majordomo()

    try: 
        while md.cycle(): pass
    except KeyboardInterrupt: pass
    finally: del md


class Majordomo:
    def __init__(self):
        self.ID = '/tmp/MDqueue'

        outQueueManager = QueueManager(address=('/tmp/SVSqueue'), \
            authkey='gnarf')
        outQueueManager.connect()

        inQueueManager = QueueManager(address=(self.ID), \
            authkey='gnarf')
        inQueueManager.connect()

        self.inQueue = inQueueManager.get_queue()
        self.outQueue = outQueueManager.get_queue()

        self.handleMsg = msgHandler()
        self.handleMsg.register('privateMsg', self.do_privateMsg)
        self.handleMsg.register('lobbyMsg', self.do_lobbyMsg)
        self.handleMsg.register('alert', self.do_alert)
        self.handleMsg.register('userJoined', self.do_userJoined)
        self.handleMsg.register('userLeft', self.do_userLeft)
        self.handleMsg.register('bye', self.do_bye)
        self.handleMsg.register('userList', self.do_userList)
        self.handleMsg.register('syslog', self.do_syslog)

        self.handleCmd = msgHandler(self.logfunc)
        self.handleCmd.register('userList', self.do_cmd_userList, 2)
        self.handleCmd.register('master', self.do_cmd_master, 3)
        self.handleCmd.register('shutdown', self.do_cmd_shutdown, 2)
        self.handleCmd.register('log', self.do_cmd_log, 2)

        self.userList = []
        self.masterList = []
        self.logList = []

        self.keep_alive = True

    def __del__(self):
        del self.inQueue
        del self.handleMsg
        del self.handleMsg
        del self.handleCmd
        del self.userList
        del self.masterList
        del self.logList

    def cycle(self):
        msg = self.inQueue.get() # blocking get
        self.performInQueueMsg(msg)

        return self.keep_alive

    def sendSystem(self, msgtag, text, obj=None):
        self.outQueue.put((msgtag, text, obj))

    def performInQueueMsg(self, msg):
        if not self.handleMsg.performMsg(msg):
            print '[Majordomo] unknown cmd: ' + msg[0]

    def logfunc(self, msg):
        self.sendSystem('syslog', str(msg), self.ID)

    def do_privateMsg(self, param):
        msg = param[1]
        sourceID = param[2]['ID']
        msgList = msg.split()
        msgList.append(sourceID)

        if not self.handleCmd.performMsg(msgList):
            self.sendSystem('chatPrivate', \
                'You know I am a bot, right?', 
                (self.ID, sourceID))

    def do_lobbyMsg(self, param): pass

    def do_alert(self, param):
        msg = param[1]
        self.sendSystem('chatLobby', msg, self.ID)

    def do_userJoined(self, param):
        userInfo = param[2]
        self.sendSystem('chatLobby', userInfo['name'] + ' joined us', \
            self.ID)
        self.sendSystem('getUserList', 'gimme all you know', self.ID)

    def do_userLeft(self, param):
        userID = param[2]
        for user in self.userList:
            if user['ID'] == userID:
                self.sendSystem('chatLobby', user['name'] + ' left us', \
                    self.ID)
        self.sendSystem('getUserList', 'gimme all you know', self.ID)

    def do_bye(self, param):
        self.keep_alive = False

    def do_userList(self, param):
        self.userList = [x for x in param[2] if x['ID'] != self.ID]

    def do_syslog(self, param):
        msgtag, msg, obj = param
        for user in self.logList:
            self.sendSystem('chatPrivate', msg + ' ' + obj, \
                (self.ID, user))

# callbacks for user commands
    def do_cmd_userList(self, param):
        sourceID = param[1]
        if sourceID not in self.masterList: return

        self.sendSystem('chatPrivate', 'I know about those users:', \
            (self.ID, sourceID))
        for user in self.userList:
            msg = user['name'] + ' (' + user['ID'] + ')'
            if user['ID'] == sourceID: msg += ', you'
            if user['ID'] in self.masterList: msg += ', my master'
            self.sendSystem('chatPrivate', msg, \
                (self.ID, sourceID))

    def do_cmd_master(self, param):
        password = param[1]
        sourceID = param[2]
        passwordHash = md5.md5(password).digest()
        if passwordHash == SVStaticData.majordomoHash:
            self.masterList.append(sourceID)
            self.sendSystem('chatPrivate', 'You are my master!', \
                (self.ID, sourceID))

    def do_cmd_shutdown(self, param):
        sourceID = param[1]
        if sourceID not in self.masterList: return

        self.sendSystem('shutdown', 'system shutdown')

    def do_cmd_log(self, param):
        sourceID = param[1]
        if sourceID not in self.masterList: return

        if sourceID in self.logList:
            self.logList.remove(sourceID)
            self.sendSystem('chatPrivate', 'deactivated log messages', \
                (self.ID, sourceID))
        else:
            self.logList.append(sourceID)
            self.sendSystem('chatPrivate', 'activated log messages', \
                (self.ID, sourceID))
