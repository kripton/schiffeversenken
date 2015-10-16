class SVError(Exception): pass

class InitializationError(SVError):
    def __init__(self, msg):
        self.msg = msg
