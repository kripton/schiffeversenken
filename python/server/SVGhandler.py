from SVGame import SVGame

class SVGhandler:
    def __init__(self):
        self.gameList = []
        self.gameID = 0

    def __del__(self):
        del self.gameList
        del self.gameID

    def addGame(self, hostID):
        newGame = SVGame(self.gameID, hostID)
        self.gameList.append(newGame)

        self.gameID += 1

    def getActiveGames(self):
        return [game for game in self.gameList \
                    if len(game.getPlayerList() == 2)]

    def getOpenGames(self):
        return [game for game in self.gameList \
                    if len(game.getPlayerList() == 1)]

    def getAllGames(self):
        return self.gameList

    def performMsg(self, msg):
        pass
