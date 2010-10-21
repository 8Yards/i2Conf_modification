/*
 *   i2conf - SIP based conference server
 *   Copyright (C) 2008-2009  Guillem Cabrera
 *
 *   This file is part of i2conf.
 *
 *   i2conf is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   i2conf is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *    Authors: Guillem Cabrera <guillem.cabrera@i2cat.net>
 */

#include "timeoutController.h"

using namespace std;

TimeoutController::TimeoutController(map<string,MRef<Room*> > rooms, Manager * sm, MRef<SipStack*> sipStack) {
	this->rooms = rooms;
	this->sm = sm;
	this->sipStack = sipStack;
}

void TimeoutController::run() {
	sleep(5);
	while(true) {
		list<string> callIds2remove;
		map<string,MRef<Room*> >::iterator it;
		for(it = rooms.begin(); it != rooms.end(); ++it) {
			
			map <string,MRef<Participant*> >::iterator it2;
			map <string,MRef<Participant*> > part = (*it).second->getParticipants();
			
			for(it2 = part.begin(); it2 != part.end(); ++it2) {
				
				list<MRef<Flow*> >::iterator it3;
				list<MRef<Flow*> > flows = (*it2).second->getFlowsFrom();
				
				for(it3 = flows.begin(); it3 != flows.end(); ++it3) {
					
					time_t now;
					time(&now);
					
					if((now-20) > sm->readTimeStamp((*it3)->getId())) {
						callIds2remove.push_back((*it2).second->getCallId());
					}
				}
			}
		}
		
		callIds2remove.unique();
		for(list<string>::iterator it4 = callIds2remove.begin(); it4 != callIds2remove.end(); ++it4) {
			CommandString cmd((*it4), "delete");
			sipStack->handleCommand(cmd);
		}
		
		sleep(10);
	}
}
