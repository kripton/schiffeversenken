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

#include "user.h"

User::User() : id(-1), status(0), client(false), letzterPong(time(NULL)), letzterPing(time(NULL))
{
}


User::~User()
{
}

int User::setid(int newid) {
	if (id >= 0) return(-1);
	id = newid;
	return(0);
}

int User::getstatus() {
	return status;
}

int User::setname(char* newname) {
	if (status > 0) return(-1); // Name wurde bereits vergeben?
	
	status = 1;
	name = newname;
	
	return(0);
}

string User::getname() {
	return name;
}

int User::getid() {
	return id;
}

void User::setclient() {
	client = true;
}

bool User::getclient() {
	return client;
}

void User::setlastping() {
	letzterPing = time(NULL);
}

void User::setlastpong() {
	letzterPong = time(NULL);
}

int User::getresponsetime() {
	return (static_cast<int>(letzterPing) - static_cast<int>(letzterPong));
}

vector<int>& User::getrooms() {
	return(tische);
}

bool User::hasroom(int tisch) {
	for (vector<int>::iterator iter = tische.begin(); iter != tische.end(); iter++) {
		if (*iter == tisch) return true;
	}
	return false;
}
