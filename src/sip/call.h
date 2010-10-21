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
 *             Erik Eliasson <eliasson@it.kth.se>
 */

#ifndef CALL_H
#define CALL_H
#include<libmsip/SipStack.h>
#include<libmsip/SipDialog.h>
#include <libminisip/signaling/sdp/SdpPacket.h>

class App;
class Room;
using namespace std;

class Call : public SipDialog{
	public:
		Call(MRef<SipStack*> stack, MRef<SipIdentity*> ident, string cid, MRef<App*> app);

		string getName();
		string getUri();
		
		bool start_inConference (const SipSMCommand &cmd);
		bool start_terminated_INVITE(const SipSMCommand &cmd);
		bool inConference_waitingForByeResponse_BYEOut (const SipSMCommand &cmd);
		bool waitingForByeResponse_terminated_200 (const SipSMCommand &cmd);
		bool anyState_terminated(const SipSMCommand &cmd);
		bool anyState_terminated_cmd(const SipSMCommand &cmd);		

	private:
		MRef<SipIdentity*> myIdentity;
		MRef<App*> app;
		MRef<Room*> room;
		MRef<SdpPacket*> sdpInPacket;
};
#include "../app.h"
#include "../rooms/room.h"
#endif
