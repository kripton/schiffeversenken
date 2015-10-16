import map

class ShipDirection:
    HORIZONTAL = 'h'
    VERTICAL = 'v'

    def typeOf(var):
        if var == ShipDirection.HORIZONTAL or \
                var == ShipDirection.VERTICAL:
            return(True)
        else:
            return(False)


class Ship:
    def __init__(self, length):
        self._length = length
        self._coords = (0, 0)
        self._direction = ShipDirection.HORIZONTAL
        self._placed = False
        
    def mapinsert(self, themap):
        if not ShipDirection.typeOf(this._direction):
            raise ValueError('Ship: direction wrong type')

        row, col = self._coords
        for i in range(self._length):
            themap._map[row][col] = 's'
            if self._direction == ShipDirection.HORIZONTAL:
                col += 1
            else:
                row += 1

    def place(self, themap, coords, direction):
        if not ShipDirection.typeOf(direction):
            raise ValueError('Ship: direction wrong type')

        row, col = coords
        self._placed = True
        for i in range(self._length):
            self._placed = self._placed and \
                    self._checkspace(themap, (row, col))
            if direction == ShipDirection.HORIZONTAL:
                col += 1
            else:
                row += 1

        if self._placed:
            self._coords = coords
            self._direction = direction
        return self._placed
            
    def _checkspace(self, themap, coords):
        row, col = coords
        for rows in range(max(0, row-1), min(9, row+2)):
            for cols in range(max(0, col-1), min(9, col+2)):
                if themap._map[rows][cols] != '.':
                    return False
        return True

    def isPlaced(self):
        return(self._placed)
