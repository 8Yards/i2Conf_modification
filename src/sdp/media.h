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

#ifndef MEDIA_H
#define MEDIA_H

#include<libminisip/signaling/sdp/SdpHeaderM.h>
#include<libminisip/signaling/sdp/SdpHeaderC.h>

#include <list>
#include <string>

#include "codec.h"

using namespace std;

class Media : public MObject {
	public:
		Media(string media, int port, string transport, string ip);
		
		string getMedia();
		void setMedia(string media);
		
		string getIp();
		void setIp(string ip);
		
		int getPort();
		void setPort(int port);
		
		string getTransport();
		void setTransport(string transport);
		
		void addCodec(MRef<Codec*> codec);
		void delCodec(string media);
		list<MRef<Codec*> > getCodecs();
		list<string> getCodecsId();
		string getCodecsIdStr();
		
		void addAttribute(string build);
		
		string getSdpMString();
		MRef<SdpHeaderM*> getSdpMObject();
		string getSdpCString();
		MRef<SdpHeaderC*> getSdpCObject();
	
	private:
		string media;
		int port;
		string transport;
		list<MRef<Codec*> > codecs;
		string ip;
		
		MRef<SdpHeaderC*> sdpC;
		MRef<SdpHeaderM*> sdpM;	
};
#endif
