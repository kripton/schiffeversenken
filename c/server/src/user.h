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
#ifndef USER_H
#define USER_H

#include <time.h>
#include <string>
#include <vector>

#include "schiffeversenkend.h"

using namespace std;

/**
	@author Gunther Janek <g.janek@boerser.de>
*/
class User{
	private:
		int id;
		string name;		// Name des Spielers
		vector<int> tische;	// Liste der Spieltische
		/* status:
		 * 0 = Warte auf Namen
		 * 1 = Lobby
		 * 2 = Warte auf Gegner (Tisch für Standardspiel eröffnet
		 * 3 = Positionieren: erwarte Koordinate
		 * 4 = Positionieren: erwarte Richtung
		 * 5 = Warte auf Spielstart
		 * 6 = Spiel läuft
		 */
		int status;
		bool client;
		time_t letzterPing;
		time_t letzterPong;
	
	
	public:
		User();
		~User();
		
		int setid(int);
		int getstatus();
		int setname(char*);
		string getname();
		int getid();
		void setclient();
		bool getclient();
		void setlastping();
		void setlastpong();
		int getresponsetime();
		vector<int>& getrooms();
		bool hasroom(int);

};

#endif
