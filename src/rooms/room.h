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

#ifndef ROOM_H
#define ROOM_H

#include "../sip/callIn.h"
#include "../sdp/sdpDesc.h"
#include "participant.h"

#include <libmutil/MemObject.h>
#include <libminisip/signaling/sdp/SdpPacket.h>

#include <map>

class streamManager;
class App;
using namespace std;

const MRef<Room*> NULL_ROOM = NULL;

class Room : public MObject {
	public:
		Room(string threadId, string conversationId, string description, MRef<App*> app);
		
		void addParticipant(string uri, string callId, MRef<SdpPacket*> sdp);
		void delParticipant(string callId);
		bool existsParticipant(string callId);
		map <string,MRef<Participant*> > getParticipants();

		void authorize(string uri);
		void unauthorize(string uri);
		bool isAuthorized(string uri);

		string getId();
		string getThreadId();
		string getRcl();

		string getConversationId();
		void setConversationId(string conversationId);
		
		string getDescription();
		void setDescription(string id);
		
		MRef<SdpDesc*> createNewSdpDesc();

		MRef<SdpDesc*> getSdp();
//		void setSdp(MRef<SdpDesc*> sdp);

		static string getRoomId(string threadId, string conversationId);

		MRef<SipMessageContentMime*> getMimePacket();
		void setMimePacket(MRef<SipMessageContentMime*> mimePacket);

	private:
		string threadId, conversationId, description;
		map <string,int> authUris;
			/*
			**	URI		Allowed
			**	alice@i2cat.net	0	->>>> Not authorized
			**	*@minisip.org	1	->>>> Authorized
			*/
		map <string,MRef<Participant*> > participants;
		MRef<SipMessageContentMime*> mimePacket;
		MRef<SdpDesc*> fixedSdp;
		MRef<App*> app;

};
#include <libstrmanager/Manager.h>
#include "../app.h"
#endif
