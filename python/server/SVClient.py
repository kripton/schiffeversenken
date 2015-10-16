import socket
import multiprocessing
import time
from QueueManager import QueueManager
from msgHandler import msgHandler
from StateMachine import StateMachine
from SVStaticData import SVStaticData
from SVError import InitializationError


def processWorker(queuepath, sock):
    client = SVClient(queuepath, sock)

    try:
        while client.cycle(): pass
    except KeyboardInterrupt: pass
    finally: del client


class SVClient:
    def __init__(self, queuepath, sock):
        self.ID = queuepath

        outQueueManager = QueueManager(address=('/tmp/SVSqueue'), \
            authkey='gnarf')
        outQueueManager.connect()

        inQueueManager = QueueManager(address=(queuepath), \
            authkey='gnarf')
        inQueueManager.connect()

        self.inQueue = inQueueManager.get_queue()
        self.outQueue = outQueueManager.get_queue()
        self.sock = sock

        self.inBuffer = ''

        self.handleMsg = msgHandler(self.logfunc)
        self.handleMsg.register('sock', self.do_sock)
        self.handleMsg.register('lobbyMsg', self.do_lobbyMsg)
        self.handleMsg.register('clientNameAccepted', \
            self.do_clientNameAccepted)
        self.handleMsg.register('clientNameDenied', \
            self.do_clientNameDenied)
        self.handleMsg.register('sendPONG', self.do_sendPONG)
        self.handleMsg.register('userList', self.do_userList)
        self.handleMsg.register('bye', self.do_bye)
        self.handleMsg.register('privateMsg', self.do_privateMsg, 3, False)
        self.handleMsg.register('userLeft', self.do_userLeft)
        self.handleMsg.register('userJoined', self.do_userJoined)

        self.handleProtIn = msgHandler()
        self.handleProtIn.register(210, self.do_210_in_proposeName, 2)
        self.handleProtIn.register(401, self.do_401_in_listRooms, 1)
        self.handleProtIn.register(406, self.do_406_in_newRoom, 1)
        self.handleProtIn.register(501, self.do_501_in_chatLobby)
        self.handleProtIn.register(502, self.do_502_in_chatPrivate)
        self.handleProtIn.register(505, self.do_505_in_getUserList, 1)
        self.handleProtIn.register(602, self.do_602_in_ackProt, 2)
        self.handleProtIn.register(603, self.do_603_in_PING, 1)
        self.handleProtIn.register(604, self.do_604_in_PONG, 1)
        self.handleProtIn.register(699, self.do_699_in_exit, 1)

        self.handleProtOut = msgHandler()
        self.handleProtOut.register(101, self.do_ProtOut_trivial)
        self.handleProtOut.register(302, self.do_ProtOut_trivial)
        self.handleProtOut.register(303, self.do_ProtOut_trivial)
        self.handleProtOut.register(305, self.do_ProtOut_trivial)
        self.handleProtOut.register(306, self.do_ProtOut_trivial)
        self.handleProtOut.register(503, self.do_503_out_chatLobby)
        self.handleProtOut.register(504, self.do_504_out_chatPrivate)
        self.handleProtOut.register(506, self.do_ProtOut_trivial)
        self.handleProtOut.register(507, self.do_507_out_userEntry)
        self.handleProtOut.register(508, self.do_ProtOut_trivial)
        self.handleProtOut.register(509, self.do_509_out_userJoined)
        self.handleProtOut.register(510, self.do_510_out_userLeft)
        self.handleProtOut.register(601, self.do_601_out_serverHello)
        self.handleProtOut.register(603, self.do_603_out_PING)
        self.handleProtOut.register(604, self.do_ProtOut_trivial)
        self.handleProtOut.register(697, self.do_ProtOut_trivial)

        self.FSMsymbols = {}
        self.FSM = StateMachine(self.FSMsymbols)
        self.FSM.add_state('init', self.st_init)
        self.FSM.set_state('init')
        self.FSM.add_state('prot_proposed', self.st_prot_proposed)
        self.FSM.add_state('prot_accepted', self.st_prot_accepted)
        self.FSM.add_state('name_asked', self.st_name_asked)
        self.FSM.add_state('online', self.st_online)

        self.userList = []

        self.firstCycle = True
        self.keep_alive = True

    def __del__(self):
        del self.inQueue
        del self.outQueue
        del self.sock
        del self.inBuffer
        del self.handleMsg
        del self.FSMsymbols
        del self.FSM

    def cycle(self):
        if self.firstCycle: self.firstCycle = False
        else: 
            msg = self.inQueue.get() # blocking get
            self.performInQueueMsg(msg)
        self.FSM.cycle()

        return self.keep_alive

    def sendPeerRaw(self, text):
        self.sock.send(text)

    def sendSystem(self, msgtag, text, obj=None):
        self.outQueue.put((msgtag, text, obj))

    def sendProt(self, ProtNo, comment, data=None):
        if not self.handleProtOut.performMsg((ProtNo, comment, data)):
            print '[SVClient.sendProt] unknown ProtNo: ' + str(ProtNo)

    def performInQueueMsg(self, msg):
        if not self.handleMsg.performMsg(msg):
            print '[SVClient] unknown cmd: ' + msg[0]

    def logfunc(self, msg):
        self.sendSystem('syslog', str(msg), self.ID)

# callbacks for msg handling: param is (msgtag, text, obj)
    def do_sock(self, param):
        self.inBuffer += param[1]
        self.checkInBuffer()

    def do_lobbyMsg(self, param):
        if self.FSM.is_state('online'):
            self.sendProt(503, 'message written in lobby', param)

    def do_clientNameAccepted(self, param):
        self.FSMsymbols['clientNameAccepted'] = 1

    def do_clientNameDenied(self, param):
        self.FSMsymbols['clientNameDenied'] = param[1]

    def do_sendPONG(self, param):
        self.sendProt(604, 'PONG')

    def do_userList(self, param):
        self.userList = param[2]
        self.sendProt(506, 'start of user list')
        for user in self.userList:
            self.sendProt(507, 'user entry', user)
        self.sendProt(508, 'end of user list')

    def do_bye(self, param):
        self.keep_alive = False

    def do_privateMsg(self, param):
        self.sendProt(504, 'private message', param)

    def do_userLeft(self, param):
        userID = param[2]
        msg = param[1]
        if userID in [x['ID'] for x in self.userList if x['ID'] == userID]:
            userName = [x['name'] for x in self.userList \
                if x['ID'] == userID][0]
            self.sendProt(510, msg, userName)
        
        self.userList = [x for x in self.userList if x['ID'] != userID]

    def do_userJoined(self, param):
        userEntry = param[2]
        self.userList.append(userEntry)
        self.sendProt(509, 'user joined', userEntry['name'])

# callbacks for protocol input handling: param is [ProtNo, (par)*]
    def do_210_in_proposeName(self, param):
        if not self.FSM.is_state('name_asked'):
            self.sendProt(306, 'not waiting for client name')
            return

        self.FSMsymbols['clientNameProposed'] = param[1]

    def do_401_in_listRooms(self, param):
        if not self.FSM.is_state('online'):
            self.sendProt(306, 'not ready for games')
            return
        self.sendSystem('listRooms', 'list open games', self.ID)

    def do_406_in_newRoom(self, param):
        if not self.FSM.is_state('online'):
            self.sendProt(306, 'not ready for games')
            return
        self.sendSystem('newRoom', 'create new game', self.ID)

    def do_501_in_chatLobby(self, param):
        if not self.FSM.is_state('online'):
            self.sendProt(306, 'not ready for chat')
            return
        tList = param
        del tList[0]
        msg = " ".join(param)
        self.sendSystem('chatLobby', msg, self.ID)

    def do_502_in_chatPrivate(self, param):
        if not self.FSM.is_state('online'):
            self.sendProt(306, 'not ready for chat')
            return
        tList = param
        if len(tList) < 2:
            self.sendProt(303, 'no username given')
            return
        del tList[0]
        sinkName = tList[0]
        del tList[0]
        msg = " ".join(tList)

        sinkID = [x['ID'] for x in self.userList if x['name'] == sinkName]
        if len(sinkID) == 0:
            self.sendProt(303, 'requested user not found')
            return
        self.sendSystem('chatPrivate', msg, (self.ID, sinkID[0]))

    def do_505_in_getUserList(self, param):
        self.sendSystem('getUserList', 'tellme all users', self.ID)

    def do_602_in_ackProt(self, param):
        if not self.FSM.is_state('prot_proposed'):
            self.sendProt(306, 'not waiting for client protocol')
            return

        ProtNo = param[1]

        if ProtNo.isdigit():
            self.FSMsymbols['clientProt'] = int(ProtNo)
        else:
            self.sendProt(302, 'waiting for protocol version')

    def do_603_in_PING(self, param):
        self.sendSystem('PINGreceived', 'ping from peer', self.ID)

    def do_604_in_PONG(self, param):
        self.sendSystem('PONGreceived', 'pong from peer', self.ID)

    def do_699_in_exit(self, param):
        self.sendSystem('exit', 'going home... goodbye', self.ID)
        sys.exit(0)

# callbacks for protocol output handling: param is [ProtNo, (par)*]
    def do_ProtOut_trivial(self, param):
        ProtNo, comment, data = param
        msg = str(ProtNo)
        if len(comment)>0: msg += ' -- ' + comment
        msg += '\n'
        self.sendPeerRaw(msg)

    def do_503_out_chatLobby(self, param):
        ProtNo, comment, data = param
        msg = str(ProtNo)
        sendername = data[2]['name']
        chat = data[1]
        msg += ' ' + sendername + ': ' + chat
        if len(comment)>0: msg += ' -- ' + comment
        msg += '\n'
        self.sendPeerRaw(msg)

    def do_504_out_chatPrivate(self, param):
        ProtNo, comment, data = param
        msg = str(ProtNo)
        sendername = data[2]['name']
        chat = data[1]
        msg += ' ' + sendername + ': ' + chat
        if len(comment)>0: msg += ' -- ' + comment
        msg += '\n'
        self.sendPeerRaw(msg)

    def do_507_out_userEntry(self, param):
        ProtNo, comment, data = param
        msg = str(ProtNo)
        name = data['name']
        msg += ' ' + name
        if len(comment)>0: msg += ' -- ' + comment
        msg += '\n'
        self.sendPeerRaw(msg)

    def do_509_out_userJoined(self, param):
        ProtNo, comment, data = param
        msg = str(ProtNo)
        name = data
        msg += ' ' + name
        if len(comment)>0: msg += ' -- ' + comment
        msg += '\n'
        self.sendPeerRaw(msg)
       
    def do_510_out_userLeft(self, param):
        ProtNo, comment, data = param
        msg = str(ProtNo)
        name = data
        msg += ' ' + name
        if len(comment)>0: msg += ' -- ' + comment
        msg += '\n'
        self.sendPeerRaw(msg)

    def do_601_out_serverHello(self, param):
        ProtNo, comment, data = param
        msg = str(ProtNo)
        protV = str(data)
        msg += ' ' + protV
        if len(comment)>0: msg += ' -- ' + comment
        msg += '\n'
        self.sendPeerRaw(msg)

    def do_603_out_PING(self, param):
        ProtNo, comment, data = param
        msg = str(ProtNo)
        if len(comment)>0: msg += ' -- ' + comment
        msg += '\n'
        self.sendPeerRaw(msg)
        self.sendSystem('PINGsent', 'ping sent to peer', self.ID)

# callbacks for FSM
    def st_init(self, symbols):
        self.sendProt(601, 'Schiffeversenken Prot V' + \
            str(SVStaticData.protocolVersion), \
            SVStaticData.protocolVersion)
        return('prot_proposed')

    def st_prot_proposed(self, symbols):
        if 'clientProt' in symbols:
            if symbols['clientProt'] == SVStaticData.protocolVersion:
                self.sendProt(603, 'PING')
                return('prot_accepted')

            else:
                self.sendProt(697, 'Protocol mismatch. Bye')
                sys.exit(0)
                #TODO: shutdown process here
        else:
            return None

    def st_prot_accepted(self, symbols):
        # PONG from peer shall cause call of this state
        self.sendProt(101, 'select your name, max 15 chars (210)')
        return('name_asked')

    def st_name_asked(self, symbols):
        if 'clientNameProposed' in symbols and \
                'clientNameForwarded' not in symbols:
            self.sendSystem('nameProposed', \
                symbols['clientNameProposed'], self.ID)
            symbols['clientNameForwarded'] = 1
            return None

        if 'clientNameDenied' in symbols:
            self.sendProt(305, symbols['clientNameDenied'])
            self.sendProt(101, 'select your name, max 15 chars (210)')
            del symbols['clientNameProposed']
            del symbols['clientNameDenied']
            del symbols['clientNameForwarded']
            return None

        if 'clientNameAccepted' in symbols:
            self.name = symbols['clientNameProposed']

            self.sendSystem('getUserList', '', self.ID)
            return('online')

    def st_online(self, symbols): pass

# helper functions
    def checkInBuffer(self):
        if '\r' in self.inBuffer or '\n' in self.inBuffer:
            bufList = [x for x in \
                self.inBuffer.replace('\r', '\n').split('\n') \
                if len(x) > 0]
            if len(bufList) > 0:
                newMsg = bufList[0]
                del bufList[0]
                self.inBuffer = '\n'.join(bufList)
                self.parsePeerMsg(newMsg)

            else:
                self.inBuffer = ''
                self.sendProt(303, 'invalid data')

    def parsePeerMsg(self, msg):
        grepMsg = msg.split(' -- ')[0]
        catList = [x for x in grepMsg.split(' ') if len(x) > 0]

        if len(catList) == 0:
            self.sendProt(303, 'invalid data')
            return

        if not catList[0].isdigit():
            self.sendProt(303, 'protcol mismatch')
            return

        ProtNo = int(catList[0])
        catList[0] = ProtNo
        if not self.handleProtIn.performMsg(catList):
            self.sendProt(303, str(ProtNo) + \
                ': invalid protocol number or wrong number of arguments')

