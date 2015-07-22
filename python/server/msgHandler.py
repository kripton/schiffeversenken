class msgHandler:
    def __init__(self):
        self.cmds = {}

    def __del__(self):
        del self.cmds

    def register(self, cmd, callback, numPars=0):
        self.cmds[cmd] = (callback, numPars)

    def performMsg(self, msg):
        cmd = msg[0]
        if cmd in self.cmds:
            callback = self.cmds[cmd][0]
            numPars = self.cmds[cmd][1]
            if (numPars == 0) or (len(msg) == numPars):
                callback(msg)
                return True

            else: return False

        else: return False
