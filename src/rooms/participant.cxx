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

#include "participant.h"

//FLOW
Flow:: Flow(int id,string ip,int port,string mediaType) : id(id), ip(ip), port(port), mediaType(mediaType) {
	
}
int Flow::getId() {
	return id;
}
string Flow::getIp() {
	return ip;
}
int Flow::getPort() {
	return port;
}
string Flow::getMediaType() {
	return mediaType;
}

//TOFLOW
ToFlow::ToFlow(int idSrc,int id,string ip,int port,string mediaType) : Flow(id,ip,port,mediaType), idSrc(idSrc) {
	
}
int ToFlow::getIdSrc() {
	return idSrc;
}

//ENDPOINT
EndPoint::EndPoint(string mediaType, string ip, int port) : mediaType(mediaType), ip(ip), port(port) {
}
string EndPoint::getMediaType() {
	return mediaType;
}
string EndPoint::getIp() {
	return ip;
}
int EndPoint::getPort() {
	return port;
}

//PARTICIPANT
Participant::Participant(string callId, Manager* sm) : callId(callId), sm(sm) {
	
}
void Participant::addFlowTo(MRef<ToFlow*> flow) {
	flowsTo.push_back(flow);
	sm->addDestination(flow->getIdSrc() ,flow->getId(), (char *) flow->getIp().c_str(), flow->getPort());
}

void Participant::delFlowTo(MRef<ToFlow*> flow) {
	sm->removeDestination(flow->getIdSrc(),flow->getId());
	flowsTo.remove(flow);
}

void Participant::addFlowFrom(MRef<Flow*> flow) {
	flowsFrom.push_back(flow);
	sm->addSource(4,flow->getId(), (char *) flow->getIp().c_str(), flow->getPort());
}

void Participant::delFlowFrom(MRef<Flow*> flow) {
	
	sm->removeSource(flow->getId());
	flowsFrom.remove(flow);
}

list<MRef<Flow*> > Participant::getFlowsFrom() {
	return flowsFrom;
}

list<MRef<ToFlow*> > Participant::getFlowsTo() {
	return flowsTo;
}

void Participant::addContactInfo(MRef<EndPoint*> endpoint) {
	contactInfo.push_back(endpoint);
}

list<MRef<EndPoint*> > Participant::getContactInfo() {
	return contactInfo;
}
bool Participant::isMediaTypeCI(string mediaType) {
	bool ret = false;
	for(list<MRef<EndPoint*> >::iterator it = contactInfo.begin(); it != contactInfo.end(); ++it) {
		if((*it)->getMediaType() == mediaType) {
			ret = true;
		}
	}
	
	return ret;
}
string Participant::getIp4Media(string mediaType) {
	string ip = "";
	for(list<MRef<EndPoint*> >::iterator it = contactInfo.begin(); it != contactInfo.end(); ++it) {
		if((*it)->getMediaType() == mediaType) {
			ip = (*it)->getIp();
		}
	}
	return ip;
}

string Participant::getCallId() {
	return callId;
}

