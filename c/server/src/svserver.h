/***************************************************************************
 *   Copyright (C) 2009 by Gunther Janek   *
 *   g.janek@boerser.de   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef SVSERVER_H
#define SVSERVER_H

#include <map>
#include <string>
#include <string.h>
#include <cstdlib>
#include <cstdio>
#include <vector>

#include "user.h"
#include "schiffeversenkend.h"
#include "chat.h"
#include "room.h"

/**
Grundlegende Serverfunktionen wie Kommunikation, Message-Parsing, User-Verwaltung...

	@author Gunther Janek <g.janek@boerser.de>
*/
class SVServer{
	private:
		map<int, User> userlist;
		map<int, Room> roomlist;
		int lastroom;
		
	public:
		SVServer();
		~SVServer();
	
		void addclient(int);
		void remclient(int);
		int sendcode(int, int);
		int processline(int, char*);
		vector<int> pingclients();

};

#endif
