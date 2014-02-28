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

#include <iostream>

#include "svserver.h"


using namespace std;

SVServer::SVServer() : lastroom(0)
{
	User newuser;
	newuser.setid(0);
	newuser.setname("Skipper");
	userlist[0] = newuser;
}


SVServer::~SVServer()
{
}

void SVServer::addclient(int id) {
	User newuser;
	userlist[id] = newuser;
#ifdef DEBUG
	cout<<"User ID "<<id<<" angelegt."<<endl;
#endif
	sendmessage(id, "Willkommen bei Gunthers Schiffeversenken.\n");
	sendcode(id, 601);
	sendcode(id, 101);
}

void SVServer::remclient(int id) {
	if (userlist[id].getstatus() > 0)
		Chat::notice_usergone(userlist[id], userlist);
	userlist.erase(id);
#ifdef DEBUG
	cout<<"User ID"<<id<<" aus dem Speicher entfernt."<<endl;
#endif
}

vector<int> SVServer::pingclients() {
	vector<int> pingret;
	
	for (map<int, User>::iterator i = userlist.begin(); i != userlist.end(); i++) {
		if (i->second.getclient()) {

			if (i->second.getresponsetime() > 90) {
				pingret.push_back(i->first);
				continue;
			}
			sendcode(i->first, 603);
			i->second.setlastping();
		}
	}
	
	return(pingret);
}

int SVServer::processline(int id, char* instruction) {
	// Nachricht parsen
	char tmp[strlen(instruction)+1];
	strcpy(tmp, instruction);
	
	if (tmp[strlen(tmp)-2]<20) tmp[strlen(tmp)-2] = '\0'; // evtl. vorh. lf/cr löschen
	if (tmp[strlen(tmp)-1]<20) tmp[strlen(tmp)-1] = '\0';
	
#ifdef DEBUG
	cout<<"ID "<<id<<": \""<<tmp<<"\""<<endl;
#endif
	
	char* p1;
	int c1 = (int)strtol(tmp, &p1, 0); // bis zu 3 int-Zahlen am Anfang lesen
	int c2=0, c3=0, c4=0;
	if (c1) {
		c2 = (int)strtol(p1, &p1, 0);
		if (c2) c3 = (int)strtol(p1, &p1, 0);
		if (c3) c4 = (int)strtol(p1, &p1, 0);
	}
	
	while ((*p1 == ' ') || (*p1 == '-')) p1++; // bis Beginn von evtl. vorh. Text rücken
	char* p2;
	if (p2 = strstr(p1, " -- ")) *p2 = '\0'; // Kommentar abschneiden
	while (p1[strlen(p1)-1] == ' ') p1[strlen(p1)-1] = '\0'; // chop
	
#ifdef DEBUG
	cout<<"Parse:"<<endl;
	cout<<"\tc1: "<<c1<<endl<<"\tc2: "<<c2<<endl<<"\tc3: "<<c3<<endl;
	cout<<"\tc4: "<<c4<<endl<<"\t\""<<p1<<"\""<<endl;
#endif
	
	// Fehler bei ungültiger Eingabe
	if (c1 == 0) {
		sendcode(id, 303);
		return(0);
	}
	
	// Solange kein Name angegeben wurde, sind nur bestimmte Anweisungen möglich
	if (userlist[id].getstatus() == 0) {
		if (!((c1 == 210) || (c1 == 602) || (c1 == 603) || (c1 == 604) || (c1 == 699))) {
			sendcode(id, 306); // Unerwartete Anweisung
			return(0);
		}
	}
	
	// Eingegangene Nachricht bearbeiten
	string message;
	Room newroom;
	switch (c1) {
		case 210: // Name eingegeben
			if (userlist[id].getstatus() > 0) {
				// Du hast schon einen Namen
				sendcode(id, 306);
				return(0);
			}
			
			if ((strlen(p1) < 3) || (strlen(p1) > 15)) {
				sendcode(id, 303); // ungültige Eingabe
				sendcode(id, 101); // Name eingeben
				return(0);
			}
			
			for (map<int, User>::iterator i = userlist.begin(); i != userlist.end(); i++) {
				if (i->second.getname() == p1) {
					sendcode(id, 305); // Name bereits vergeben
					sendcode(id, 101); // Name eingeben
					return(0);
				}
			}
			
			userlist[id].setname(p1);
			Chat::notice_newuser(userlist[id], userlist);
			return(0);
			break;
			
		case 406: // Neuen Raum eröffnen
			newroom.setplayer1(id);
			return(0);
			break;
			
		case 501: // In den Chat schreiben
			message = p1;
			Chat::chat(userlist[id], c2, message, userlist, roomlist);
			return(0);
			break;
			
		case 505: // Benutzerliste ausgeben
			sendcode(id, 506); // Anfang Liste
			for (map<int, User>::iterator i = userlist.begin(); i != userlist.end(); i++) {
				if (i->first == 0) continue; // Skipper nicht mit aufführen
				
				message = "507 ";
				message += i->second.getname();
				message += "\n";
				sendmessage(id, message.c_str());
			}
			sendcode(id, 508); // Ende Liste
			break;
			
		case 602: // Bestätigung des Protokolls (Anmeldung als Software)
			if (c2 != PROTOCOL_VERSION) {
				sendcode(id, 306); // Anweisung in diesem Kontext nicht anwendbar
				remclient(id); // Falsche Clientversion wird vorsichtshalber gekickt
				return(-1);
			}
			userlist[id].setclient();
			return(0);
			break;
			
		case 603: // Ping
			sendcode(id, 604); // Pong
			userlist[id].setlastping();
			userlist[id].setlastpong();
			return(0);
			break;
		
		case 604: // Pong
			userlist[id].setlastpong();
			return(0);
		
		case 699: // Abmelden
			remclient(id);
			return(-1); // Meldung an selectserver, den Socket zu löschen
			break;
		
		default: // Ungültiger Code
			sendcode(id, 303); // ungültige Eingabe
	}
	
	return 0;
}

int SVServer::sendcode(int id, int code) {
	int retval;
	string message;
	bool sendOK = true;
	char temp[5] = "";
	
	switch(code) {
		case 101:
			message = "101 -- Bitte gib deinen Namen ein (mit Code 210, 3-15 Zeichen)\n";
			break;
		case 102:
			message = "102 -- Gib die Startkoordinaten des Schiffs ein (mit Code 211)\n";
			break;
		case 103:
			message = "103 -- Gib die Richtung zum positionieren ein (mit Code 212)\n";
			break;
		case 104:
			message = "104 -- Gib die Schusskoordinaten ein (mit Code 213)\n";
			break;
		
		case 201:
			message = "201 -- Eigenes Spielfeld\n";
			break;
		case 202:
			message = "202 -- Gegnerisches Spielfeld\n";
			break;
		case 203:
			message = "203 -- Kein Treffer\n";
			break;
		case 204:
			message = "204 -- Treffer\n";
			break;
		case 205:
			message = "205 -- Versenkt\n";
			break;
		case 206:
			message = "206 -- Spielbeginn\n";
			break;
		case 207:
			message = "207 -- Du hast gewonnen\n";
			break;
		case 208:
			message = "208 -- Du hast verloren\n";
			break;
		case 209:
			message = "209 -- Spielende\n";
			break;
			
		case 301:
			message = "301 -- Ungueltige Koordinate. Eingabe wiederholen.\n";
			break;
		case 302:
			message = "302 -- Plausibilitaetspruefung nicht bestanden. Eingabe wiederholen.\n";
			break;
		case 303:
			message = "303 -- Ungueltige Eingabe\n";
			break;
		case 304:
			message = "304 -- Feld wurde schon beschossen. Eingabe wiederholen.\n";
			break;
		case 305:
			message = "305 -- Name bereits vergeben. Eingabe wiederholen.\n";
			break;
		case 306:
			message = "306 -- Anweisung in diesem Kontext nicht anwendbar.\n";
			break;
		
		case 506:
			message = "506 -- Anfang der Benutzerliste\n";
			break;
		case 508:
			message = "508 -- Ende der Benutzerliste\n";
			break;
		
		case 601:
			//message = "601 PROTOCOL_VERSION -- Schiffeversenken Protokoll V. PROTOCOL_VERSION\n";
			snprintf(temp, 4, "%d", PROTOCOL_VERSION);
			message = "601 ";
			message += temp;
			message += " -- Schiffeversenken Protokoll V. ";
			message += temp;
			message += "\n";
			break;
		case 603:
			message = "603 -- Ping\n";
			break;
		case 604:
			message = "604 -- Pong\n";
			break;
		case 698:
			message = "698 -- Ping Timeout\n";
			break;
		
		default:
			sendOK = false;
	}
	
	if (sendOK) retval = sendmessage(id, message.c_str());
	else retval = -1;
	
	return (retval);
}

