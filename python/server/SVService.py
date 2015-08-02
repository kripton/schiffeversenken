import socket
import select
import threading
import multiprocessing
import Majordomo
import time
from QueueManager import QueueManager
from msgHandler import msgHandler
from SVGhandler import SVGhandler


def processWorker():
    service = SVService()

    try:
        while service.cycle(): pass
    except KeyboardInterrupt: pass
    finally: del service


class SVService:
    def __init__(self):
        inQueueManager = QueueManager(address=('/tmp/SVSqueue'), \
            authkey='gnarf')
        inQueueManager.connect()
        self.inQueue = inQueueManager.get_queue()

        self.handleMsg = msgHandler(self.logfunc)
        self.handleMsg.register('init', self.do_init)
        self.handleMsg.register('addClient', self.do_addClient)
        self.handleMsg.register('peerQuit', self.do_peerQuit)
        self.handleMsg.register('exit', self.do_exit)
        self.handleMsg.register('procDead', self.do_procDead)
        self.handleMsg.register('chatLobby', self.do_chatLobby)
        self.handleMsg.register('nameProposed', self.do_nameProposed)
        self.handleMsg.register('PINGsent', self.do_PINGsent)
        self.handleMsg.register('PINGreceived', self.do_PINGreceived)
        self.handleMsg.register('PONGreceived', self.do_PONGreceived)
        self.handleMsg.register('getUserList', self.do_getUserList)
        self.handleMsg.register('chatPrivate', self.do_chatPrivate, 3, False)
        self.handleMsg.register('shutdown', self.do_shutdown)
        self.handleMsg.register('syslog', self.do_syslog, 3, False)

        self.clients = {}
        self.lastClient = 0

        self.SVG = SVGhandler()

        self.init = False
        self.keep_alive = True

    def __del__(self):
        del self.inQueue
        del self.handleMsg
        del self.clients
        #if self.MDproc.is_alive():
            #self.sendClient(self.MDqueuepath, 'bye', 'system shutdown')
        self.MDproc.join()
        self.mqm.shutdown()

    def cycle(self):
        msg = self.inQueue.get() # blocking get
        self.performInQueueMsg(msg)

        if msg[0] == 'bye':
            print 'SVService: Bye'
            return False

        return self.keep_alive

    def sendClient(self, ID, msgtag, text, obj=None):
        msg = (msgtag, text, obj)
        self.clients[ID]['queue'].put((msgtag, text, obj))

    def getSenderInfo(self, ID):
        return {'name': self.clients[ID]['name'],
                'ID': ID}

    def removeClient(self, clientID, msg):
        clientName = self.clients[clientID]['name']
        del self.clients[clientID]['name']
        del self.clients[clientID]['queue']
        del self.clients[clientID]['state']
        del self.clients[clientID]

        for ID in self.clients.keys():
            if not self.clients[ID]['name'][0] == '_':
                self.sendClient(ID, 'userLeft', msg, clientID)

    def performInQueueMsg(self, msg):
        if not self.handleMsg.performMsg(msg):
            print '[SVService] unknown cmd: ' + msg[0]

    def logfunc(self, msg):
        self.sendClient(self.MDqueuepath, 'syslog', 'event from SVService', str(msg))

    def do_init(self, param):
        if not self.init:
            self.MDqueuepath = '/tmp/MDqueue'
            self.mqm = QueueManager(address=(self.MDqueuepath), \
                authkey='gnarf')
            self.mqm.start()
            self.MDqueue = self.mqm.get_queue()

            self.clients[self.MDqueuepath] = { \
                'name': 'Majordomo',
                'queue': self.MDqueue,
                'state': 'master'}

            self.MDproc = multiprocessing.Process( \
                target=Majordomo.processWorker,
                args=[])
            self.MDproc.start()

            self.init = True

    def do_addClient(self, param):
        self.lastClient += 1
        queuepath = param[2] # we use path to named pipe as client id

        m = QueueManager(address=(queuepath), authkey='gnarf')
        m.connect()
        queue = m.get_queue()

        self.clients[queuepath] = { \
            'name': '_SVuser' + str(self.lastClient),
            'queue': queue,
            'state': 'init'}

    def do_peerQuit(self, param):
        clientID = param[2]
        if clientID in self.clients:
            self.removeClient(clientID, 'peer disconnected')
        
    def do_exit(self, param):
        clientID = param[2]
        if clientID in self.clients:
            self.removeClient(clientID, 'user left us')

    def do_procDead(self, param):
        clientID = param[2]
        if clientID in self.clients:
            self.removeClient(clientID, \
                'process dead - this should never happen')

    def do_chatLobby(self, param):
        text = param[1]
        senderID = param[2]

        for ID in self.clients.keys():
            self.sendClient(ID, 'lobbyMsg', text, \
                self.getSenderInfo(senderID))

    def do_nameProposed(self, param):
        proposedName = param[1]
        clientID = param[2]

        if proposedName in [x['name'] for x in self.clients.values()]:
            self.sendClient(clientID, 'clientNameDenied', \
                'a user with this name is already online')

        elif len(proposedName) > 15:
            self.sendClient(clientID, 'clientNameDenied', \
                'name too long')

        elif len(proposedName) < 3:
            self.sendClient(clientID, 'clientNameDenied', \
                'name too short')

        elif proposedName[0] == '_':
            self.sendClient(clientID, 'clientNameDenied', \
                'name must not begin with an underscore')

        elif  ':' in proposedName or ' ' in proposedName:
            self.sendClient(clientID, 'clientNameDenied', \
                'name must not contain : or space')

        else: 
            self.clients[clientID]['name'] = proposedName
            self.sendClient(clientID, 'clientNameAccepted', \
                proposedName)
            for sink in self.clients.keys():
                if not self.clients[sink]['name'][0] == '_':
                    self.sendClient(sink, 'userJoined', 'user joined', \
                        self.getSenderInfo(clientID))

    def do_PINGsent(self, param): pass

    def do_PINGreceived(self, param):
        clientID = param[2]
        self.sendClient(clientID, 'sendPONG', 'send PONG to peer')

    def do_PONGreceived(self, param): pass

    def do_getUserList(self, param):
        clientID = param[2]
        userList = [self.getSenderInfo(x) \
            for x in self.clients.keys() 
            if (not self.clients[x]['name'][0] == '_') or
            (clientID == self.MDqueuepath)]
        self.sendClient(clientID, 'userList', '', userList)

    def do_chatPrivate(self, param):
        msg = param[1]
        sourceID, sinkID = param[2]

        self.sendClient(sinkID, 'privateMsg', msg, \
            self.getSenderInfo(sourceID))

    def do_shutdown(self, param):
        self.sendClient(self.MDqueuepath, 'bye', 'system shutdown')

        self.keep_alive = False

    def do_syslog(self, param):
        msgtag, msg, userID = param
        self.sendClient(self.MDqueuepath, 'syslog', 'event from ' + userID, msg)
