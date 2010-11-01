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

	MRef<TimeoutController*> tc = new TimeoutController(rooms,sm,*sipStack);
//	Thread tController(*tc);
}

void App::handleCommand(string subsystem, const CommandString &cmd) {
	
}

CommandString App::handleCommandResp(string subsystem, const CommandString &cmd) {
	CommandString ret;
	return ret;
}

bool App::handleCommand(const SipSMCommand& cmd) {
	massert(sipStack);
	
	/**
    	* To avoid mcu from crash, check cmd contains any
    	* commandPacket or just the String (Error message)
    	*/
	if ( cmd.getType()!=SipSMCommand::COMMAND_PACKET )
		return false;
	
	if (cmd.getCommandPacket()->getType()=="INVITE") {   //Act as a server
		MRef<SipDialog*> call = new Call(sipStack,
						myIdentity, 
						cmd.getCommandPacket()->getCallId(), 
						this);		
		lastCallId = call->getCallId();
		calls[lastCallId] = dynamic_cast<Call*> (*call);
		sipStack->addDialog(call);
		bool ret = call->handleCommand(cmd);
		//massert(ret);
		return ret;
	}
	else {
		cerr << "App: I don't know how to handle packet " << cmd.getCommandPacket()->getType() << endl;
		return false;
	}
	
	return true;
}


void App::run() {
	if(sipRegister) {
		MRef<SipDialog*> reg = new SipDialogRegister(sipStack,myIdentity);
		sipStack->addDialog(*reg);
		reg->handleCommand(SipSMCommand(CommandString(reg->getCallId(),SipCommandString::proxy_register), SipSMCommand::dialog_layer, SipSMCommand::dialog_layer));
	}
	
	//Run the SipStack
	sipStack->run();
}

string App::getLastCallId() {
	return lastCallId;
}

void App::removeCallId(string callId) {
	calls.erase(callId);
}

MRef<Room* > App::getRoom(string id) {
	return rooms[id];
}
map<string, MRef<Room*> > App::getRooms() {
	return rooms;
}
void App::addRoom(string id, string description) {
	rooms[id] = new Room(id,description,this);
}

string App::readRoom(string toUri) {

	string::size_type pos = toUri.find(";room="); //type4-64bits
	
	if(pos == toUri.npos) {
		return "default";
	}
	
	string room = toUri.substr(pos+6);
	
	pos = room.find(";");
	if(pos != room.npos) {
		room = room.substr(0,pos);
	}
	
	pos = room.find(">");
	if(pos != room.npos) {
		room = room.substr(0,pos);
	}
	
	return room;
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
	return mediaPort-2;
}

string App::getUri(string callId) {
	return calls[callId]->getUri();
}

void App::loadConfig(string configFile) {
	
	//read data from the XML config file
	MRef<XMLConfig*> xmlc = new XMLConfig(configFile);
	
	useUdp = xmlc->readBool("use_udp",true);
	useTcp = xmlc->readBool("use_tcp",false);
	string interface = xmlc->readString("interface","eth0");
		
	sipPort = xmlc->readInt("sip_identity/local_port",5060);
	string uri = xmlc->readString("sip_identity/uri","");
	sipRegister = xmlc->readBool("sip_identity/register",false);
	string user = xmlc->readString("sip_identity/username","");
	string passwd = xmlc->readString("sip_identity/password","");
	string realm = xmlc->readString("sip_identity/realm","");
	string proxy_address = xmlc->readString("sip_identity/proxy_address",realm);
	string proxy_protocol = xmlc->readString("sip_identity/proxy_protocol","udp");
	int proxy_port = xmlc->readInt("sip_identity/proxy_port",5060);
	
	//SipStack config
	MRef<SipStackConfig*> config = new SipStackConfig;
	config->preferedLocalSipPort = sipPort;
	config->preferedLocalSipsPort = 0;
	config->localIpString=NetworkFunctions::getInterfaceIPStr(interface);
	sipStack = new SipStack(config);
	sipStack->setDebugPrintPackets(true);
	sipStack->setDefaultDialogCommandHandler(this);
	sipStack->setCallback(this); //set this dialog as the default callback
	sipStack->setTransactionHandlesAck(true); //get the ACK messages in the Dialog Layer
		
	//start the servers
	if (!(useUdp||useTcp)) {
		useUdp = true;
	}
	if (useUdp){
		sipStack->startUdpServer();
	}
	if (useTcp){
		sipStack->startTcpServer();
	}
	
	//set the user identity
	string proto;
	if(useTcp) {
		proto = "TCP";
	}
	if(useUdp) {
		proto = "UDP";
	}
	
	myIdentity= new SipIdentity(SipUri(uri));
	myIdentity->setSipRegistrar(new SipRegistrar(SipUri(uri).getIp(), proto));
	
	if(sipRegister) {
		string proxy = realm;
		if(proxy_address != realm) {
			proxy = proxy_address;
			myIdentity->setSipProxy(false, uri, proxy_protocol, proxy, proxy_port);
		}
		
		myIdentity->setCredential(new SipCredential(user,passwd,proxy));
	}
	
	//In order to deal with sdp, we need a factory
	SipMessage::contentFactories.addFactory("application/sdp", sdpSipMessageContentFactory);
	
	
	//add default room
	MRef<Room*> defaultRoom = new Room("default", "Public conference room", this);
	
	//authorize all sip uris in the conference
	defaultRoom->authorize("*");
	
	//Create SDP description for the room
	defaultRoom->setSdp(new SdpDesc(0, "i2conf", sipStack->getStackConfig()->localIpString, "Conference " + defaultRoom->getId() + ": " + defaultRoom->getDescription(), "", 0, 0));	
	
	//Add audio media
	MRef<Media*> audio = new Media("audio", /*getMediaPort()*/5568, "RTP/AVP", sipStack->getStackConfig()->localIpString);
		//speex
		MRef<Codec*> codec = new Codec("Speex", "119", "speex");
		codec->addAttribute("16000");
		audio->addCodec(codec);
		
		//PCMU
		codec = new Codec("G711 PCMU", "0", "PCMU");
		codec->addAttribute("8000");
		codec->addAttribute("1");
		audio->addCodec(codec);
		
		//PCMA
		codec = new Codec("G711 PCMA", "8", "PCMA");
		codec->addAttribute("8000");
		codec->addAttribute("1");
		audio->addCodec(codec);
	
	defaultRoom->getSdp()->addMedia(audio);
	
	MRef<Media*> video = new Media("video", /*getMediaPort()*/ 5566, "RTP/AVP", sipStack->getStackConfig()->localIpString);
		
		//H264
		codec = new Codec("H264", "99", "H264");
		codec->addAttribute("90000");
		video->addCodec(codec);
		
		video->addAttribute("fmtp:99 profile-level-id=644028");
		
	defaultRoom->getSdp()->addMedia(video);
	
	
	
	rooms["default"] = defaultRoom;
}
