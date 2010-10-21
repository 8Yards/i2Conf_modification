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

#include"codec.h"

using namespace std;

Codec::Codec(string name, string mediaTypeId, string sdpName) {
	this->name = name;
	this->mediaTypeId = mediaTypeId;
	this->sdpName = sdpName;
	sdpA = new SdpHeaderA("a=rtpmap:" + getSdpAString());
}

string Codec::getName() {
	return name;
}
void Codec::setName(string name) {
	this->name = name;
}

string Codec::getMediaTypeId() {
	return mediaTypeId;
}
void Codec::setMediaTypeId(string mt) {
	mediaTypeId = mt;
	sdpA = new SdpHeaderA("a=rtpmap:" + getSdpAString());
}

string Codec::getSdpName() {
	return sdpName;
}
void Codec::setSdpName(string sdpName) {
	this->sdpName = sdpName;
	sdpA = new SdpHeaderA("a=rtpmap:" + getSdpAString());
}

void Codec::addAttribute(string att) {
	attributes.push_back(att);
	sdpA = new SdpHeaderA("a=rtpmap:" + getSdpAString());
}
void Codec::delAttribute(string att) {
	attributes.remove(att);
	sdpA = new SdpHeaderA("a=rtpmap:" + getSdpAString());
}
list<string> Codec::getAttributes() {
	return attributes;
}

string Codec::getSdpAString() {
	string sdpAString = mediaTypeId + " " + sdpName;
	for(list<string>::iterator it = attributes.begin(); it != attributes.end(); ++it) {
		sdpAString = sdpAString + "/" + (*it);
	}
	return sdpAString;
}

MRef<SdpHeaderA*> Codec::getSdpAObject() {
	return sdpA;
}
