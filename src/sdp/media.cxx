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

#include "media.h"

#include <list>

using namespace std;

Media::Media(string media, int port, string transport, string ip) {
	this->media = media;
	this->port = port;
	this->transport = transport;
	this->ip = ip;
	
	sdpC = new SdpHeaderC("IN", "IP4", ip);
	sdpM = new SdpHeaderM(media,port,1,transport);
	sdpM->setConnection(*getSdpCObject());
	
}

string Media::getMedia() {
	return media;
}
void Media::setMedia(string media) {
	this->media = media;
}

string Media::getIp() {
	return ip;
}
void Media::setIp(string ip) {
	this->ip = ip;
}

int Media::getPort() {
	return port;
}
void Media::setPort(int port) {
	this->port = port;
}

string Media::getTransport() {
	return transport;
}
void Media::setTransport(string transport) {
	this->transport = transport;
}

void Media::addCodec(MRef<Codec*> codec) {
	codecs.push_back(codec);
	sdpM->addFormat(codec->getMediaTypeId());
	sdpM->addAttribute(codec->getSdpAObject());
}

void Media::delCodec(string media) {
	list<MRef<Codec*> > toRemove;
	for(list<MRef<Codec*> >::iterator it = codecs.begin(); it != codecs.end(); ++it) {
		if((*it)->getMediaTypeId() == media) {
			toRemove.push_back((*it));
		}
	}
	
	for(list<MRef<Codec*> >::iterator it = toRemove.begin(); it != toRemove.end(); ++it) {
		codecs.remove(*it);
	}
}
list<MRef<Codec*> > Media::getCodecs() {
	return codecs;
}
list<string> Media::getCodecsId() {
	list<string> codecsR;
	for(list<MRef<Codec*> >::iterator it = codecs.begin(); it != codecs.end(); ++it) {
		codecsR.push_back((*it)->getMediaTypeId());
	}
	return codecsR;
}
string Media::getCodecsIdStr() {
	string codecsList;
	for(list<MRef<Codec*> >::iterator it = codecs.begin(); it != codecs.end(); ++it) {
		codecsList = codecsList + " " + (*it)->getMediaTypeId();
	}
	return codecsList;
}

void Media::addAttribute(string build) {
	sdpM->addAttribute(new SdpHeaderA("a=" + build));
}

string Media::getSdpMString() {	 
	return sdpM->getString();
}
MRef<SdpHeaderM*> Media::getSdpMObject() {
	return sdpM;
}
string Media::getSdpCString() {
	return sdpC->getString();
}
MRef<SdpHeaderC*> Media::getSdpCObject() {
	return sdpC;
}
