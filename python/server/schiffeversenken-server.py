class map:
	def __init__(self):
		self._map = self._emptymap()

	def _emptyline(self):
		return(['.' for self._i_el in range(10)])

	def _emptymap(self):
		return([self._emptyline() for self._i_em in range(10)])

	def get_map(self):
		return(self._map)

	def get_mapstring(self):	# TODO: verschiedene funktionen private/enemy
		self._mapstring = '  1 2 3 4 5 6 7 8 9 10\n';
		self._charCount = ord('A')

		for self._line in self._map:
			self._mapstring += chr(self._charCount) + ' '
			self._charCount += 1

			for self._line_elem in self._line:
				self._mapstring += self._line_elem + ' '
			self._mapstring += '\n'
		return(self._mapstring)

	def encodeCoords(self, index):
		if type(index) is not tuple:
			raise ValueError('expecting a tuple of 2 indices')
		elif len(index) != 2:
			raise ValueError('expecting a tuple of 2 indices')

		self._row, self._col = index
		if type(self._row) is not int or type(self._col) is not int:
			raise ValueError('the 2 indices must be of type int')
		elif self._row < 0 or self._row > 9 \
				or self._col < 0 or self._col > 9:
			raise ValueError('index bounds exceeded')

		self._rowCode = chr(ord('A') + self._row)
		self._colCode = str(self._col + 1)
		return(self._rowCode + self._colCode)
		
	def decodeCoords(self, coordString):
		if isinstance(coordString, str):
			if len(coordString) == 2:
				self._rowCode, self._colCode = coordString
			elif len(coordString) == 3:
				self._rowCode = coordString[0]
				self._colCode = coordString[1:3]
			else:
				raise ValueError('coord string broken (wrong len)')

			if 		ord(self._rowCode) < ord('A') or \
					ord(self._rowCode) > ord('J') or \
					int(self._colCode) < 1 or \
					int(self._colCode) > 10:
				raise ValueError('coord string broken' + self._rowCode, self._colCode)
			else:
				self._coordIndex = ord(self._rowCode) - ord('A') , \
						int(self._colCode) - 1

			return(self._coordIndex)

		else:
			raise ValueError('expecting string containing coords')



class ship:
	def __init__(self, length):
		self._length = length
		self._coords = (0, 0)
