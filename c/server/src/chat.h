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
#ifndef CHAT_H
#define CHAT_H

#include <map>
#include <iostream>

#include "schiffeversenkend.h"
#include "user.h"
#include "room.h"

using namespace std;

/**
	@author Gunther Janek <g.janek@boerser.de>
*/
class Chat{
	public:
		Chat();
		~Chat();
		
		static void notice_newuser(User, map<int, User>&);
		static void notice_usergone(User, map<int, User>&);
		static void chat(User, int, string, map<int, User>&, map<int, Room>&);

};

#endif
