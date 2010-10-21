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

#ifndef CODEC_H
#define CODEC_H

#include<libminisip/signaling/sdp/SdpHeaderA.h>

#include <list>
#include <string>

using namespace std;

class Codec : public MObject {
	public:
		Codec(string name, string mediaTypeId, string sdpName);
		
		string getName();
		void setName(string name);
		
		string getMediaTypeId();
		void setMediaTypeId(string mt);
		
		string getSdpName();
		void setSdpName(string sdpName);
		
		void addAttribute(string att);
		void delAttribute(string att);
		list<string> getAttributes();
		
		string getSdpAString();
		MRef<SdpHeaderA*> getSdpAObject();
	
	private:
		string name;
		string mediaTypeId;
		string sdpName;
		MRef<SdpHeaderA*> sdpA;
		list<string> attributes;	
};
#endif
