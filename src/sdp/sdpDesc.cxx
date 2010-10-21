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

#include "sdpDesc.h"

using namespace std;

SdpDesc::SdpDesc(int version, string owner, string ip, string name, string information, int startTime, int endTime) {
	this->ip = ip;
	setV(version);
	setO(owner + " - - IN IP4 " + ip);
	setSessionName(name);
	setSessionInformation(information);
	setTime(startTime,endTime);
}
		
MRef<SdpPacket*> SdpDesc::getSdpPacket() {
	MRef<SdpPacket*> pkt = new SdpPacket();
	
	if(!version) {
		pkt->addHeader(dynamic_cast<SdpHeader*> (*getV()));
	}
	
	if(getOStr() != "") {
		pkt->addHeader(dynamic_cast<SdpHeader*> (*getO()));
	}
	
	if(getSessionNameStr() != "") {
		pkt->addHeader(dynamic_cast<SdpHeader*> (*getSessionName()));
	}
	
	if(getSessionInformationStr() != "") {
		pkt->addHeader(dynamic_cast<SdpHeader*> (*getSessionInformation()));
	}
	
	if(getTimeStr() != "") {
		pkt->addHeader(dynamic_cast<SdpHeader*> (*getTime()));
	}
	
	for(list<MRef<Media*> >::iterator it = media.begin(); it != media.end(); ++it) {
		pkt->addHeader(dynamic_cast<SdpHeader*> (*((*it)->getSdpMObject())));
	}
	
	
	
	return pkt;
}

int SdpDesc::getVStr() {
	return version;
}
MRef<SdpHeaderV*> SdpDesc::getV() {
	return new SdpHeaderV("v=" + getVStr());
}
void SdpDesc::setV(int v) {
	version = v;
}

string SdpDesc::getOStr() {
	return ownerCreator;
}
MRef<SdpHeaderO*> SdpDesc::getO() {
	return new SdpHeaderO("o=" + getOStr());
}
void SdpDesc::setO(string o) {
	ownerCreator = o;
}

string SdpDesc::getSessionNameStr() {
	return sessionName;
}
MRef<SdpHeaderS*> SdpDesc::getSessionName() {
	return new SdpHeaderS("s=" + getSessionNameStr());
}
void SdpDesc::setSessionName(string sessionName) {
	this->sessionName = sessionName;
}

string SdpDesc::getSessionInformationStr() {
	return sessionInformation;
}
MRef<SdpHeaderI*> SdpDesc::getSessionInformation() {
	return new SdpHeaderI("i=" + getSessionInformationStr());
}
void SdpDesc::setSessionInformation(string sessionInformation) {
	this->sessionInformation = sessionInformation;
}

string SdpDesc::getTimeStr() {
	return itoa(startTime) + " " + itoa(endTime);;
}
MRef<SdpHeaderT*> SdpDesc::getTime() {
	return new SdpHeaderT("t=" + getTimeStr());
}
int SdpDesc::getStartTime() {
	return startTime;
}
int SdpDesc::getEndTime() {
	return endTime;
}
void SdpDesc::setTime(int startTime, int endTime) {
	this->startTime = startTime;
	this->endTime = endTime;
}

void SdpDesc::addMedia(MRef<Media*> media) {
	this->media.push_back(media);
}

list<MRef<Media*> > SdpDesc::getMedia() {
	return media;
}
