class msgHandler:
    def __init__(self, logfunc=None):
        self.cmds = {}
        self.logfunc = logfunc

    def __del__(self):
        del self.cmds

    def register(self, cmd, callback, numPars=0, syslog=True):
        self.cmds[cmd] = (callback, numPars, syslog)

    def performMsg(self, msg):
        cmd = msg[0]
        if cmd in self.cmds:
            callback, numPars, syslog = self.cmds[cmd]
            if (numPars == 0) or (len(msg) == numPars):
                callback(msg)

                if self.logfunc and syslog:
                    self.logfunc(msg)
                return True

            else: return False

        else: return False
