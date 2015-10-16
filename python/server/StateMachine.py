from SVError import InitializationError

class StateMachine:
    def __init__(self, symbols):
        self.states = {}
        self.symbols = symbols
        self.state = None

    def __del__(self):
        del self.states
        del self.symbols
        del self.state

    def add_state(self, name, callback):
        name = name.upper()
        if name in self.states:
            raise InitializationError('[set_state] state ' + name + \
                ' already present')
        else:
            self.states[name] = callback

    def set_state(self, name):
        name = name.upper()
        if name in self.states:
            self.state = name
        else:
            raise InitializationError("[set_state] state " + name + \
                ' not defined')

    def get_state(self):
        return self.state

    def is_state(self, name):
        name = name.upper()
        return name == self.state

    def cycle(self):
        if not self.state in self.states:
            raise InitializationError('[cycle] set FSM to proper' + \
                ' state before cycling')
                
        newState = self.states[self.state](self.symbols)
        # callbacks can either return 'None' or the new state's name
        if newState:
            self.set_state(newState.upper())
