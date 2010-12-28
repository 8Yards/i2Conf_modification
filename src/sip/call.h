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
/*
 * Authors: Prajwol Kumar Nakarmi <prajwolkumar.nakarmi@gmail.com>
 * 			Nina Mulkijanyan <nmulky@gmail.com>
 */

#ifndef CALL_H
#define CALL_H

#include<libmsip/SipStack.h>
#include<libmsip/SipDialog.h>
#include <libminisip/signaling/sdp/SdpPacket.h>
#include<libmsip/SipMessageContentMime.h>
#include<libmsip/SipMessageContentRCL.h>

class App;
class Room;
using namespace std;

class Call: public SipDialog {
public:
	Call(MRef<SipStack*> stack, MRef<SipIdentity*> ident, string cid,
			MRef<App*> app);

	bool start_calling_inviteOut(const SipSMCommand &cmd);
	bool start_inCall_inviteIn(const SipSMCommand &cmd);
	bool calling_inCall_200(const SipSMCommand &cmd);
	bool inCall_calling_invite(const SipSMCommand &cmd);
	bool inCall_inCall_refer(const SipSMCommand &cmd);
	bool inCall_terminated_bye(const SipSMCommand &cmd);
	bool anyState_terminated(const SipSMCommand &cmd);
	bool anyState_terminated_hangup_cmd(const SipSMCommand &cmd);
	bool callOut(const SipSMCommand &cmd, bool isReInvite);
	void releaseResources();

	string getName();
	string getUri();

	MRef<Room*> getRoom();
	void setRoom(MRef<Room*> room);

private:
	MRef<App*> app;
	MRef<Room*> room;
	MRef<SipRequest*> lastInvite;
	MRef<SipIdentity*> myIdentity;
};
#include "../app.h"
#include "../rooms/room.h"
#endif
