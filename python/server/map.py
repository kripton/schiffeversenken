import ship

class map:
    _map = []
    _ships = []

    def __init__(self):
        self._map = self._emptymap()

    def addShip(self, coords):
        pass

    def _emptyline(self):
        return(['.' for i in range(10)])

    def _emptymap(self):
        return([self._emptyline() for i in range(10)])

    def get_map(self):
        return(self._map)

    def get_mapstring(self):	# TODO: unterscheide private/enemy
        mapstring = '  1 2 3 4 5 6 7 8 9 10\n';
        charCount = ord('A')

        for line in self._map:
            mapstring += chr(charCount) + ' '
            charCount += 1

            for line_elem in line:
                mapstring += line_elem + ' '
            mapstring += '\n'
        return(mapstring)

    def encodeCoords(self, index):
        if type(index) is not tuple:
            raise ValueError('expecting a tuple of 2 indices')
        elif len(index) != 2:
            raise ValueError('expecting a tuple of 2 indices')

        row, col = index
        if type(row) is not int or type(col) is not int:
            raise ValueError('the 2 indices must be of type int')
        elif row < 0 or row > 9 \
               or col < 0 or col > 9:
            raise ValueError('index bounds exceeded')

        rowCode = chr(ord('A') + row)
        colCode = str(col + 1)
        return(rowCode + colCode)
		
    def decodeCoords(self, coordString):
        if isinstance(coordString, str):
            if len(coordString) == 2:
                rowCode, colCode = coordString
            elif len(coordString) == 3:
                rowCode = coordString[0]
                colCode = coordString[1:3]
            else:
                raise ValueError('coord string broken (wrong len)')

            if ord(rowCode) < ord('A') or ord(rowCode) > ord('J') or \
                    int(colCode) < 1 or int(colCode) > 10:
                raise ValueError('coord string broken: ' + coordString)
            else:
                coordIndex = ord(rowCode) - ord('A') , int(colCode) - 1

            return(coordIndex)

        else:
            raise ValueError('expecting string containing coords')


