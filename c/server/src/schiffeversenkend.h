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

#define DEBUG
#define PORT 9034
#define PROTOCOL_VERSION 1

int sendmessage(int, const char*);

/* Protokollspezifikation V1
 * 100er-Band: Anforderungen
 * 101 Server: Anforderung Name der(des) Spieler(s) (online/lokal)
 * 102 Server: Positionieren: Startkoordinaten eingeben
 *	"102 %d1" d1: Raumnr
 * 103 Server: Positionieren: Richtung eingeben
 *	"103 %d1" d1: Raumnr
 * 104 Server: Spiel: Schusskoordinate eingeben
 *	"104 %d1" d1: Raumnr
 * 
 * 200er-Band: Mitteilungen
 * 201 Server: Ankündigung Ausgabe des eigenen Spielfelds
 *	"201 %d1" d1: Raumnr
 * 202 Server: Ankündigung Ausgabe des gegnerischen Spielfelds
 *	"202 %d1" d1: Raumnr
 *	Auf 201 und 202 folgen 10 Zeilen mit 10 Zeichen
 * 203 Server: Kein Treffer
 *	"203 %d1" d1: Raumnr
 * 204 Server: Treffer
 *	"204 %d1" d1: Raumnr
 * 205 Server: Versenkt
 *	"205 %d1" d1: Raumnr
 * 206 Server: Spielbeginn
 *	"206 %d1" d1: Raumnr
 * 207 Server: Du hast gewonnen
 *	"207 %d1" d1: Raumnr
 * 208 Server: Du hast verloren
 *	"208 %d1" d1: Raumnr
 * 209 Server: Spielende
 *	"209 %d1" d1: Raumnr
 *	Folgt auf 207, 208 und wenn einer der Benutzer das Spiel verlassen hat
 * 210 Client: Eigener Name (auf 101)
 *	"210 %s1" 2<len(s1)<15
 * 211 Client: Startkoordinaten (auf 102)
 *	"211 %d1 %d2 %d3" d1: Tischnr, d2: Zeile, d3: Spalte
 * 212 Client: Richtung (auf 103)
 *	"212 %d1 %s1" d1: Tischnr, s1[0]: 'r'/'u'
 * 213 Client: Schusskoordinaten (auf 104)
 *	"213 %d1 %d2 %d3" d1: Tischnr, d2: Zeile, d3: Spalte
 * 214 Server: Name des Gegenspielers (nach "206 -- Spielbeginn")
 *	"214 %s1"
 * 
 * 300er-Band: Fehlermeldungen
 * 	(bei "Eingabe wiederholen folgt die Anfrage durch den Server erneut,
 *	bei ungültig/unerwartet folgt keine neue Anfrage)
 * 301 Server: Ungültige Koordinate. Eingabe wiederholen
 * 302 Server: Plausibilitätsprüfung nicht bestanden. Eingabe wiederholen
 * 303 Server: Ungültige Eingabe.
 * 304 Server: Feld wurde schon beschossen. Eingabe wiederholen
 * 305 Server: Bei Anmeldung: Name bereits vergeben. Eingabe wiederholen
 * 306 Server: Unerwartete Anweisung (Code im aktuellen Kontext nicht anwendbar)
 * 
 * 400er-Band: Raummanagement
 * 401 Client: Anforderung Liste der offenen Räume
 * 402 Server: Anfang der Raumliste
 * 403 Server: Offener Raum
 *	"403 %d1 %s1" d1: Raumnr, s1: Name des Eröffners
 * 404 Server: Ende der Raumliste
 * 405 Server: Publiziere neuen Raum
 *	"405 %d1 %s1" d1: Raumnr, s1: Name des Eröffners
 * 406 Client: eröffne neuen Raum
 * 407 Client: betrete Raum
 *	"407 %d1" d1: Raumnr
 * 408 Server: Raum betreten
 *	"408 %d1" d1: Raumnr
 * 409 Server: Raum konnte nicht betreten werden
 * 410 Server: Raum steht nicht mehr zur Verfügung
 * 411 Client: Raum verlassen (kann auch laufendes Spiel abbrechen)
 *	"411 %d1" d1: Raumnr
 * 412 Client: Anforderung Liste der laufenden Spiele
 * 413 Server: Anfang der Spiele-Liste
 * 414 Server: Laufendes Spiel
 *	"414 %s1 %s2" s1: Spieler1, s2: Spieler2
 * 415 Server: Ende der Spiele-Liste
 * 
 * 500er-Band: Chatsystem
 *	(Raumnr. 0 entspricht immer der Lobby. Ist default, wenn 501.d1 weggelassen wird.)
 * 501 Client: Nachricht schreiben
 * 	"501 %d1 - %s1" %d1: Raumnr, %s1: Nachricht
 * 502 *entfernt*
 * 503 Server: Eingehende Nachricht
 *	"503 %d1 %s1: %s2" d1: Raumnr, s1: Name d. Abs., %s2: Nachricht
 * 504 *entfernt*
 * 505 Client: Anforderung Liste der angemeldeten Spieler
 * 506 Server: Anfang der Spielerliste
 * 507 Server: Name eines angemeldeten Spielers
 *	"507 %s1" s1: Spielername
 * 508 Server: Ende der Spielerliste
 * 509 Server: Neuer Spieler online
 *	"509 %s1" s1: Spielername
 * 510 Server: Spieler offline gegangen
 *	"510 %s1" s1: Spielername
 *
 * 600er-Band: Systembefehle
 * 601 Server: Begrüßung ("Schiffeversenken Prot %d");
 *	"601 %d1" d1: Protokoll-Versionsnr.
 * 602 Client: Bestätigung des Protokolls
 *	"602 %d1" d1: Protokoll-Versionsnr.
 * 603 Ping
 * 604 Pong
 * 698 Server: Ping Timeout (und anschließender kick)
 * 699 Client: Exit
 *
 * Hinter jeder Anweisung kann ein Kommentar folgen, eingeleitet durch die
 * Zeichenfolge " -- ". Der Server macht hiervon exzessiv Gebrauch, um
 * Telnetusern das Leben zu vereinfachen. Demensprechend schneidet der
 * Parser des Servers alles nach einer solchen Zeichenfolge ab. Dies bedeutet
 * auch, dass ein Benutzer im Chat diese Zeichenfolge nicht benutzen könnte.
 */

