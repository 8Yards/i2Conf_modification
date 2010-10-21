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

#ifndef APP_H
#define APP_H

#include<libmsip/SipStack.h>
#include<libmnetutil/NetworkFunctions.h>
#include<libminisip/signaling/sdp/SdpPacket.h>

#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>

#include <libstrmanager/Manager.h>

#include<string>

#include "sip/call.h"
#include "config/XMLConfig.h"



class Room;
using namespace std;

class App : public SipDefaultHandler, public Runnable {
	public:
		App(string configFile);
		void handleCommand(string subsystem, const CommandString &cmd);
		CommandString handleCommandResp(string subsystem, const CommandString &cmd);
		bool handleCommand(const SipSMCommand& cmd);
		void run();
		string getLastCallId();
		
		MRef<SipStack*> sipStack;
		
		MRef<Room* > getRoom(string id);
		map<string, MRef<Room*> > getRooms();
		void addRoom(string id, string description);
		string readRoom(string toUri);
		void removeCallId(string callId);
		
		int getMediaPort();
		string getUri(string callId);
		
		Manager* getStreamManager();
		int getFlowId();	
		
	private:
		MRef<SipIdentity*> myIdentity;
		bool useUdp;
		bool useTcp;
		bool sipRegister;
		string lastCallId;
		int sipPort;
		int mediaPort;
		int flowId;
		
		map<string, MRef<Call*> > calls;
		map<string,MRef<Room*> > rooms;
		
		Manager* sm;
		
		void loadConfig(string configFile);
};
#include "rooms/room.h"
#endif
