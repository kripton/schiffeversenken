from map import map
from StateMachine import StateMachine

class SVGame:
    def __init__(self, gameID, hostID):
        self.gameID = gameID
        self.hostID = hostID
        self.guestID = None
        self.hostMap = map()
        self.guestMap = map()

        self.FSMsymbols = {}
        self.FSM = StateMachine(self.FSMsymbols)
        self.FSM.add_state('init', self.st_init)
        self.FSM.set_state('init')

    def __del__(self):
        del self.hostID
        del self.guestID
        del self.hostMap
        del self.guestMap
        del self.FSM
        del self.FSMsymbols

    def getID(self):
        return self.gameID

    def getPlayerList(self):
        retVal = [self.hostID]
        if self.guestID: retVal.append(self.guestID)
        return retVal

    def st_init(self, symbols):
        if 'player_apply' in symbols:
            self.guestID = symbols['player_apply']
            return ('place_ships')
