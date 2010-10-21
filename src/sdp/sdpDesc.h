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

#ifndef SDPDESC_H
#define SDPDESC_H

#include <list>

#include <libminisip/signaling/sdp/SdpPacket.h>
#include <libminisip/signaling/sdp/SdpHeaderV.h>
#include <libminisip/signaling/sdp/SdpHeaderO.h>
#include <libminisip/signaling/sdp/SdpHeaderS.h>
#include <libminisip/signaling/sdp/SdpHeaderI.h>
#include <libminisip/signaling/sdp/SdpHeaderT.h>

#include "media.h"

using namespace std;

class SdpDesc : public MObject {
	public:
		SdpDesc(int version, string owner, string ip, string name, string information, int startTime, int endTime);
		
		MRef<SdpPacket*> getSdpPacket();
		
		int getVStr();
		MRef<SdpHeaderV*> getV();
		void setV(int v);
		
		string getOStr();
		MRef<SdpHeaderO*> getO();
		void setO(string o);	
		
		string getSessionNameStr();
		MRef<SdpHeaderS*> getSessionName();
		void setSessionName(string sessionName);
		
		string getSessionInformationStr();
		MRef<SdpHeaderI*> getSessionInformation();
		void setSessionInformation(string sessionInformation);
		
		string getTimeStr();
		MRef<SdpHeaderT*> getTime();
		int getStartTime();
		int getEndTime();
		void setTime(int startTime, int endTime);
		
		void addMedia(MRef<Media*> media);
		list<MRef<Media*> > getMedia();
		
	private:
		int version;
		string ip;
		string ownerCreator;
		string sessionName;
		string sessionInformation;
		int startTime, endTime;
		list<MRef<Media*> > media;
};
#endif
