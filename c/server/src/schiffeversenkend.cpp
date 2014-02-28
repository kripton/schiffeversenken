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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <cstdlib>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <string>

#include "schiffeversenkend.h"
#include "svserver.h"

using namespace std;

int main(int argc, char *argv[]) {
	fd_set master;
	fd_set read_fds;
	struct sockaddr_in myaddr;
	struct sockaddr_in remoteaddr;
	int fdmax;
	int listener;
	int newfd;
	char buf[256];
	int nbytes;
	int yes=1;
	socklen_t addrlen;
	int i;
	int spret = 0;
	time_t lastping = 0;
	struct timeval selecttimeout;
	int selectret;
	vector<int> pingret;
	
	SVServer srv;
	
	// timeout für select, um periodisch pingen zu können
	selecttimeout.tv_usec = 0;
	selecttimeout.tv_sec = 1;
	
	// clear the master and temp sets
	FD_ZERO(&master);
	FD_ZERO(&read_fds);
	
	// get the listener
	if ((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}
	
	// loose the pesky "address already in use" error message
	if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		perror("setsockopt");
		exit(1);
	}
	
	// bind
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = INADDR_ANY;
	myaddr.sin_port = htons(PORT);
	memset(myaddr.sin_zero, '\0', sizeof myaddr.sin_zero);
	if (bind(listener, (struct sockaddr *)&myaddr, sizeof myaddr) == -1) {
		perror("bind");
		exit(1);
	}
	
	// listen
	if (listen(listener, 10) == -1) {
		perror("listen");
		exit(1);
	}
	
	// add listener to master set
	FD_SET(listener, &master);
	
	// keep track of biggest file descriptor
	fdmax = listener; // so far, it's this one
	
	cout<<"server up on port "<<PORT<<endl;
	
	// main loop
	for (;;) {
		// timeout für select, um periodisch pingen zu können
		selecttimeout.tv_usec = 0;
		selecttimeout.tv_sec = 1;
		
		read_fds = master; // copy it
		if ((selectret = select(fdmax+1, &read_fds, NULL, NULL, &selecttimeout)) == -1) {
			perror("select");
			exit(1);
		}
		
		// Alle 30 Sekunden einen Ping an alle bestätigten Clients senden
		if (time(NULL)-30 > lastping) {
			pingret.resize(0);
			//TODO:pingret = schiffeversenken(pingret, NULL, 3);
			pingret = srv.pingclients();
				
			if (!pingret.empty()) { // Gab es Ping Timeouts?
				for (vector<int>::iterator iter = pingret.begin(); iter != pingret.end(); iter++) {
					//cout<<"selectserver: Wuerde Socket "<<*iter<<" kicken.\n";
					srv.sendcode(*iter, 698); // Ping Timeout
					srv.remclient(*iter);
					close(*iter);
					FD_CLR(*iter, &master);
				}
			}
			lastping = time(NULL);
		}
		
		if (!selectret) continue; // Falls kein Eingang anstand, einfach wieder warten
		
		// run through existing connections looking for data to read
		for (i=0; i <= fdmax; i++) {
			if (FD_ISSET(i, &read_fds)) { // we got one!
				if (i == listener) {
					//handle new connections
					addrlen = sizeof remoteaddr;
					if ((newfd = accept(listener, (struct sockaddr *)&remoteaddr, &addrlen)) == -1) {
						perror("accept");
					} else {
						FD_SET(newfd, &master); // add to master set
						if (newfd > fdmax) fdmax = newfd; // keep track of the maximum
						cout<<"selectserver: new connection from ";
						cout<<inet_ntoa(remoteaddr.sin_addr);
						cout<<" on socket "<<newfd<<endl;
						
						srv.addclient(newfd);
					}
				} else {
					// handle data from a client
					if ((nbytes = recv(i, buf, sizeof(buf)-1, 0)) <= 0) {
						// got error or connection closed by client
						if (nbytes == 0) {
							//connection closed
							cout<<"selectserver: socket "<<i<<" hung up"<<endl;
						} else {
							perror("recv");
						}
						close(i); // bye!
						FD_CLR(i, &master); // remove from master set
						
						srv.remclient(i);
					} else {
						// we got some data from a client
						buf[nbytes] = '\0';
						spret = srv.processline(i, buf);
						
						if (spret == -1) {
							cout<<"selectserver: requested end of session by socket"<<i<<endl;
							sendmessage(i, "bye\n");
							
							close(i);
							FD_CLR(i, &master);
						}
					}
				} // it's SO UGLY!
			}
		}
	}
	
}

int sendmessage(int socket, const char* buf) {
	int retval = 0;
	
	if ((retval = send(socket, buf, strlen(buf), 0)) == -1) {
		perror("send");
	}
	
	return(retval);
}
