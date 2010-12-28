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

#include "app.h"
#include "rooms/room.h"
#include "timeoutController.h"

#include<libmsip/SipCommandString.h>
#include<libmsip/SipDialogRegister.h>
#include<libmnetutil/UDPSocket.h>
#include<libmutil/Exception.h>

using namespace std;

App::App(string configFile) {
	mediaPort = 20000;
	flowId = -2;
	sm = new Manager();
	loadConfig(configFile);

	MRef<TimeoutController*> tc = new TimeoutController(rooms, sm, *sipStack);
	//	Thread tController(*tc);
}

void App::handleCommand(string subsystem, const CommandString &cmd) {
}

CommandString App::handleCommandResp(string subsystem, const CommandString &cmd) {
	CommandString ret;
	return ret;
}

bool App::handleCommand(const SipSMCommand& cmd) {
	massert(sipStack)
;	/**
	 * To avoid mcu from crash, check cmd contains any
	 * commandPacket or just the String (Error message)
	 */
	if (cmd.getType() != SipSMCommand::COMMAND_PACKET) {
		return false;
	}

	if (cmd.getCommandPacket()->getType() == "INVITE") {
		MRef<Call*> call = new Call(sipStack, myIdentity, cmd.getCommandPacket()->getCallId(), this);
		lastCallId = call->getCallId();
		calls[lastCallId] = call;
		sipStack->addDialog(dynamic_cast<SipDialog*> (*call));
		bool ret = call->handleCommand(cmd);
		//massert(ret)
		return ret;
	} else {
		cerr << "App:: I don't know how to handle packet "	<< cmd.getCommandPacket()->getType() << endl;
		return false;
	}

	return true;
}

void App::run() {
	if (sipRegister) {
		MRef<SipDialog*> reg = new SipDialogRegister(sipStack, myIdentity);
		sipStack->addDialog(*reg);
		reg->handleCommand(SipSMCommand(CommandString(reg->getCallId(),
				SipCommandString::proxy_register), SipSMCommand::dialog_layer,
				SipSMCommand::dialog_layer));
	}

	//Run the SipStack
	sipStack->run();
}

string App::getLastCallId() {
	return lastCallId;
}

void App::removeCallId(string callId) {
	//	calls.erase(callId);
	calls.erase(callId);
}

MRef<Room*> App::getRoom(string threadId, string conversationId) {
	return getRoom(threadId, conversationId, false);
}

MRef<Room*> App::getRoom(string threadId, string conversationId,
		bool createIfNotExist) {
	MRef<Room*> room;
	map<string, MRef<Room*> >::iterator iter = rooms.find(Room::getRoomId(
			threadId, conversationId));

	if (iter == rooms.end() && createIfNotExist) {
		room = new Room(threadId, conversationId, "", this);
		rooms[room->getId()] = room;
		cout << "new room created" << room->getId() << endl;
	} else {
		room = iter->second;
	}

	return room;
}

map<string, MRef<Room*> > App::getRooms() {
	return rooms;
}

Manager* App::getStreamManager() {
	return sm;
}

int App::getFlowId() {
	flowId = flowId + 2;
	return flowId;
}

int App::getMediaPort() {
	mediaPort = mediaPort + 2;
	return mediaPort - 2;
}

void App::loadConfig(string configFile) {

	//read data from the XML config file
	MRef<XMLConfig*> xmlc = new XMLConfig(configFile);

	useUdp = xmlc->readBool("use_udp", true);
	useTcp = xmlc->readBool("use_tcp", false);
	string interface = xmlc->readString("interface", "eth0");

	sipPort = xmlc->readInt("sip_identity/local_port", 5060);
	string uri = xmlc->readString("sip_identity/uri", "");
	sipRegister = xmlc->readBool("sip_identity/register", false);
	string user = xmlc->readString("sip_identity/username", "");
	string passwd = xmlc->readString("sip_identity/password", "");
	string realm = xmlc->readString("sip_identity/realm", "");
	string proxy_address =
			xmlc->readString("sip_identity/proxy_address", realm);
	string proxy_protocol = xmlc->readString("sip_identity/proxy_protocol",
			"udp");
	int proxy_port = xmlc->readInt("sip_identity/proxy_port", 5060);

	//SipStack config
	MRef<SipStackConfig*> config = new SipStackConfig;
	config->preferedLocalSipPort = sipPort;
	config->preferedLocalSipsPort = 0;
	config->localIpString = NetworkFunctions::getInterfaceIPStr(interface);
	sipStack = new SipStack(config);
	sipStack->setDebugPrintPackets(false);
	sipStack->setDefaultDialogCommandHandler(this);
	sipStack->setCallback(this); //set this dialog as the default callback
	sipStack->setTransactionHandlesAck(true); //get the ACK messages in the Dialog Layer

	//start the servers
	if (!(useUdp || useTcp)) {
		useUdp = true;
	}
	if (useUdp) {
		sipStack->startUdpServer();
	}
	if (useTcp) {
		sipStack->startTcpServer();
	}

	//set the user identity
	string proto;
	if (useTcp) {
		proto = "TCP";
	}
	if (useUdp) {
		proto = "UDP";
	}

	myIdentity = new SipIdentity(SipUri(uri));
	myIdentity->setSipRegistrar(new SipRegistrar(SipUri(uri).getIp(), proto));

	if (sipRegister) {
		string proxy = realm;
		if (proxy_address != realm) {
			proxy = proxy_address;
			myIdentity->setSipProxy(false, uri, proxy_protocol, proxy,
					proxy_port);
		}

		myIdentity->setCredential(new SipCredential(user, passwd, proxy));
	}

	//In order to deal with sdp, we need a factory
	SipMessage::contentFactories.addFactory("application/sdp",
			sdpSipMessageContentFactory);
	SipMessage::contentFactories.addFactory("application/resource-lists+xml",
			SipRCLContentFactory);
}

MRef<Room*> App::replaceRoom(string threadId, string oldConvId,
		string newConvId) {
	MRef<Room*> newRoom = *getRoom(threadId, oldConvId);
	rooms.erase(newRoom->getId());

	newRoom->setConversationId(newConvId);
	rooms[newRoom->getId()] = newRoom;
	cout<< "Room replaced " << oldConvId << " to " << newConvId << endl;

	return newRoom;
}

void App::removeRoom(string roomName) {
	rooms.erase(roomName);
}

map<string, MRef<Call*> > App::getCalls() {
	return this->calls;
}

void App::addCall(MRef<Call*> call) {
	calls[call->getCallId()] = call;
}

MRef<Call*> App::getCallById(string callId) {
	return calls.find(callId)->second;
}
