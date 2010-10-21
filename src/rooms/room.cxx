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

#include "room.h"

#include<libminisip/signaling/sdp/SdpHeaderM.h>
#include<libminisip/signaling/sdp/SdpHeaderC.h>

using namespace std;

Room::Room(string id, string description, MRef<App*> app) : id(id), description(description), app(app) {

}

string Room::getId() {
	return id;
}

void Room::setId(string id) {
	this->id = id;
}

string Room::getDescription() {
	return description;
}
void Room::setDescription(string id) {
	this->description = description;
}

void Room::addParticipant(string callId, MRef<SdpPacket*> sdp) {
	
	//read the SDP media lines
	vector< MRef<SdpHeader*> > sdpHeaders = sdp->getHeaders();
	
	//Create the new participant
	MRef<Participant*> participant = new Participant(callId, app->getStreamManager());
	//set up the contact info for this participant
	
	string ip4All = "";
	for(unsigned int i = 0; i < sdpHeaders.size() ; i++) {
		
		if(!sdpHeaders[i].isNull() && (sdpHeaders[i])->getType() == SDP_HEADER_TYPE_C) {
			MRef<SdpHeaderC*> sdpCH = MRef<SdpHeaderC*>((SdpHeaderC *)*(sdpHeaders[i]));
			ip4All = sdpCH->getAddr();
		}

		if(!sdpHeaders[i].isNull() && (sdpHeaders[i])->getType() == SDP_HEADER_TYPE_M) {
			MRef<SdpHeaderM*> sdpMH = MRef<SdpHeaderM*>((SdpHeaderM *)*(sdpHeaders[i]));
			string ip4Media = "";
			if((int)sdpMH->getConnection() != 0) {
				ip4Media = sdpMH->getConnection()->getAddr();
			}
			string ip = "";
			if(ip4Media != "") {
				ip = ip4Media;
			}
			else {
				ip = ip4All;
			}
			participant->addContactInfo(new EndPoint(sdpMH->getMedia(),ip,sdpMH->getPort()));
		}	
	}
		
	//Add the flows from the new participant in the conference
	list<MRef<Media*> > roomMedia = fixedSdp->getMedia();
	for(list<MRef<Media*> >::iterator it = roomMedia.begin(); it != roomMedia.end(); ++it) {
		if(participant->isMediaTypeCI((*it)->getMedia())) {
			participant->addFlowFrom(new Flow(app->getFlowId(),
							participant->getIp4Media((*it)->getMedia()),
							(*it)->getPort(), (*it)->getMedia()));
		}
	}
	
	//go through all the participants
	for(map<string, MRef<Participant*> >::iterator it = participants.begin(); it != participants.end(); ++it) {
		
		/*if((*it).second->get) {
			continue;
		}*/
		
		//Add the other participants as a source for the flows from the new participant in the conference
		list<MRef<EndPoint*> > destinations = (*it).second->getContactInfo();
		list<MRef<Flow*> > sources = participant->getFlowsFrom();
		
		 //go through all the contact info
		for(list<MRef<EndPoint*> >::iterator it2 = destinations.begin(); it2 != destinations.end(); ++it2) {
			
			//go through all the sources of the new participant
			for(list<MRef<Flow*> >::iterator it3 = sources.begin(); it3 != sources.end() ; ++it3) { 
				
				//check if the media type is the same in the flow and in the contact info
				if((*it2)->getMediaType() == (*it3)->getMediaType()) { 
					(*it).second->addFlowTo(new ToFlow((*it3)->getId(),
								app->getFlowId(),(*it2)->getIp(),
								(*it2)->getPort(),
								(*it3)->getMediaType())); //add a new destination
				}
			}			
		}
		
		//Add the new participant as a source for the flows from the previous participants in the conference
		sources = (*it).second->getFlowsFrom();
		destinations = participant->getContactInfo();
		
		//go through all the sources of the previous participants
		for(list<MRef<Flow*> >::iterator it2 = sources.begin(); it2 != sources.end(); ++it2) {
			
			//go through all the contact info
			for(list<MRef<EndPoint*> > ::iterator it3 = destinations.begin(); it3 != destinations.end() ; ++it3) { 
			
				//check if the media type is the same in the flow and in the contact info
				if((*it2)->getMediaType() == (*it3)->getMediaType()) {
					
					participant->addFlowTo(new ToFlow((*it2)->getId(),
									app->getFlowId(),
									(*it3)->getIp(),
									(*it3)->getPort(),
									(*it3)->getMediaType())); //add a new destination
				}
			}	
		}		
		
	}
	//add the participant to room participants
	participants[callId] = participant;	
}


void Room::delParticipant(string callId) {
	
	MRef<Participant*> participant = participants[callId];
	
	if(!participant) {
		return;
	}
	
	//DELETE DESTINATIONS TO THAT PARTICIPANT
	list<MRef<ToFlow*> > deleteTo; //flowId, mediaType, SrcId
	list<MRef<ToFlow*> > flowsTo = participant->getFlowsTo();	
	for(list<MRef<ToFlow*> >::iterator it = flowsTo.begin(); it != flowsTo.end(); ++it) {
		deleteTo.push_back((*it));
	}
	
	for(list<MRef<ToFlow*> >::iterator it = deleteTo.begin(); it != deleteTo.end(); ++it) {
		participant->delFlowTo((*it));
	}
	
	list<MRef<Flow*> > flowsFrom = participant->getFlowsFrom();
	//DELETE DESTINATION TO OTHER PARTICIPANTS
	//go through all the participants in the room
	for(map<string,MRef<Participant*> >::iterator it = participants.begin() ; it != participants.end() ; ++it) { 
	
		deleteTo.clear();
		flowsTo.clear();
		
		flowsTo = (*it).second->getFlowsTo();
		for(list<MRef<ToFlow*> >::iterator it2 = flowsTo.begin(); it2 != flowsTo.end(); ++it2) {
			for(list<MRef<Flow*> >::iterator it3 = flowsFrom.begin(); it3 != flowsFrom.end(); ++it3) {
				if((*it3)->getId() == (*it2)->getIdSrc()) {
					deleteTo.push_back((*it2));
				}
			}
			
		}	
	}
	
	
	//DELETE SOURCES FROM THIS PARTICIPANT
	list<MRef<Flow*> > deleteFrom;
	//go through all the sources from that participant
	for(list<MRef<Flow*> >::iterator it = flowsFrom.begin(); it != flowsFrom.end(); ++it) { 
		deleteFrom.push_back((*it));
	}
	
	for(list<MRef<Flow*> >::iterator it = deleteFrom.begin(); it != deleteFrom.end(); ++it) {
		participant->delFlowFrom((*it));
	}	

	participants.erase(callId);
	app->removeCallId(callId);
}

map <string,MRef<Participant*> > Room::getParticipants() {
	return participants;
}

void Room::authorize(string uri) {
	authUris[uri] = 1;
}
void Room::unauthorize(string uri) {
	authUris[uri] = 0;
}
bool Room::isAuthorized(string uri) {
	
	if(authUris["*"] == 1 || authUris[uri] == 1 || authUris["*" +  uri.substr(uri.find("@"),uri.size())] == 1) {
		return true;
	}
	
	return false;
}

MRef<SdpDesc*> Room::getSdp() {
	return fixedSdp;
}
void Room::setSdp(MRef<SdpDesc*> sdp) {
	fixedSdp = sdp;
}
