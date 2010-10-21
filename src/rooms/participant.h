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

#ifndef PARTICIPANT_H
#define PARTICIPANT_H

#include <libmutil/MemObject.h>

#include <libstrmanager/Manager.h>

#include <map>
#include <string>

class Manager;
using namespace std;

class Flow : public MObject {
	public:
		Flow(int id,string ip,int port,string mediaType);
		int getId();
		string getIp();
		int getPort();
		string getMediaType();
	
	private:
		int id;
		string ip;
		int port;
		string mediaType;
};

class ToFlow : public Flow {
	public:
		ToFlow(int idSrc,int id,string ip,int port,string mediaType);
		int getIdSrc();
	
	private:
		int idSrc;	
};

class EndPoint : public MObject {
	public:
		EndPoint(string mediaType, string ip, int port);
		string getMediaType();
		string getIp();
		int getPort();
	
	private:
		string mediaType;
		string ip;
		int port;	
};

class Participant : public MObject {
	public:
		Participant(string callId, Manager* sm);
		void addFlowTo(MRef<ToFlow*> flow);
		void delFlowTo(MRef<ToFlow*> flow);
		void addFlowFrom(MRef<Flow*> flow);
		void delFlowFrom(MRef<Flow*> flow);
		list<MRef<Flow*> > getFlowsFrom();
		list<MRef<ToFlow*> > getFlowsTo();
		string getCallId();
		
		void addContactInfo(MRef<EndPoint*> endpoint);
		list<MRef<EndPoint*> > getContactInfo();
		bool isMediaTypeCI(string mediaType);
		string getIp4Media(string mediaType);
	
	private:
		string callId;
		Manager* sm;
		list<MRef<EndPoint*> > contactInfo; //<mediaType,EndPoint>
		list<MRef<ToFlow*> > flowsTo; //<mediaType,ToFlow>
		list<MRef<Flow*> > flowsFrom;//<mediaType,Flow>
};
#endif
