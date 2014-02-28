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
#include "chat.h"

Chat::Chat()
{
}


Chat::~Chat()
{
}

void Chat::notice_newuser(User newuser, map<int, User> &userlist) {
	string message;
	message = newuser.getname();
	message += " hat die Lobby betreten.";
	map<int, Room> emptyroomlist;
	chat(userlist[0], 0, message, userlist, emptyroomlist);
}

void Chat::notice_usergone(User goneuser, map<int, User> &userlist) {
	string message;
	message = goneuser.getname();
	message += " hat uns verlassen.";
	map<int, Room> emptyroomlist;
	chat(userlist[0], 0, message, userlist, emptyroomlist);
}

void Chat::chat(User poster, int room, string text, map<int, User> &userlist, map<int, Room> &roomlist) {
	string message;
	message = "503 ";
	message += poster.getname();
	message += ": ";
	message += text;
	message += "\n";
	
	//if (room > 0) {
		
	
	for (map<int, User>::iterator i = userlist.begin(); i != userlist.end(); i++) {
		if (i->first == 0) continue;
		if ((room > 0) && (!i->second.hasroom(room))) continue;
		sendmessage(i->first, message.c_str());
	}
}
