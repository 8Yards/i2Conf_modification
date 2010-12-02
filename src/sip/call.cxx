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

#include<string>

#include "call.h"
#include<libmsip/SipTransitionUtils.h>
#include<libmsip/SipHeaderMaxForwards.h>
#include<libmsip/SipHeaderCallID.h>
#include<libmsip/SipHeaderFrom.h>
#include<libmsip/SipHeaderTo.h>
#include<libmsip/SipHeaderCSeq.h>
#include<libmsip/SipHeaderContact.h>
#include<libmsip/SipHeaderUnknown.h>
#include<libmsip/SipHeaderUserAgent.h>
#include<libmsip/SipHeaderEvent.h>

#include<libmsip/SipCommandString.h>
#include "callClient.h"

using namespace std;

Call::Call(MRef<SipStack*> stack, MRef<SipIdentity*> ident, string cid, MRef<App*> app):
	SipDialog(stack, ident, cid), myIdentity(ident), app(app) {
	
	//start state
	State<SipSMCommand,string> *s_start = new State<SipSMCommand,string>(this,"start");
	addState(s_start);
	
	//inCoference state
	State<SipSMCommand,string> *s_inConference = new State<SipSMCommand,string>(this,"inConference");
	addState(s_inConference);
	
	//waitingForByeResponse state
	State<SipSMCommand,string> *s_waitingForByeResponse = new State<SipSMCommand,string>(this,"waitingForByeResponse");
	addState(s_waitingForByeResponse);
	
	//terminated state
	State<SipSMCommand,string> *s_terminated = new State<SipSMCommand,string>(this,"terminated");
	addState(s_terminated);
	
	/* added by Nina*/
	//REFER received
//	State<SipSMCommand,string> *s_refer = new State<SipSMCommand,string>(this, "refer");
//	addState(s_refer) ;

	

	//from start to inConference
	new StateTransition<SipSMCommand,string>(this,
			"transition_start_inConference",
			(bool (StateMachine<SipSMCommand,string>::*)(const SipSMCommand&)) &Call::start_inConference, 
			s_start,
			s_inConference);		
	
			
	//from start to terminated when an INVITE is received, but not all conditions are fulfilled
	new StateTransition<SipSMCommand,string>(this,
			"transition_start_terminated_INVITE",
			(bool (StateMachine<SipSMCommand,string>::*)(const SipSMCommand&)) &Call::start_terminated_INVITE, 
			s_start,
			s_terminated);
	
	//from inConference to waitingForByeResponse when a BYE is sent
	new StateTransition<SipSMCommand,string>(this,
			"transition_inConference_waitingForByeResponse_BYEOut",
			(bool (StateMachine<SipSMCommand,string>::*)(const SipSMCommand&)) &Call::inConference_waitingForByeResponse_BYEOut, 
			s_inConference, 
			s_waitingForByeResponse);
	
	//from waitingForByeResponse to terminated when a BYE response (200 OK) is received
	new StateTransition<SipSMCommand,string>(this,
			"transition_waitingForByeResponse_terminated_200",
			(bool (StateMachine<SipSMCommand,string>::*)(const SipSMCommand&)) &Call::waitingForByeResponse_terminated_200, 
			s_waitingForByeResponse, 
			s_terminated);
	
	//from anystate to terminated when a BYE or a 4XX or 6XX is received
	new StateTransition<SipSMCommand,string>(this,
			"transition_anyState_terminated",
			(bool (StateMachine<SipSMCommand,string>::*)(const SipSMCommand&)) &Call::anyState_terminated, 
			anyState, 
			s_terminated);
			
	//from anystate to terminated when a terminate command is received
	new StateTransition<SipSMCommand,string>(this,
			"transition_anyState_terminated_cmd",
			(bool (StateMachine<SipSMCommand,string>::*)(const SipSMCommand&)) &Call::anyState_terminated_cmd, 
			anyState, 
			s_terminated);

	/* added by nina */
//	from inConference to inConference refer received
	new StateTransition<SipSMCommand,string>(this,
			"transition_inConference_inConference_REFER",
			(bool (StateMachine<SipSMCommand,string>::*)(const SipSMCommand&)) &Call::handle_refer,
			s_inConference,
			s_inConference);

	/* added by nina */
//	from start to inConference refer received
	new StateTransition<SipSMCommand,string>(this,
			"transition_start_inConeference_REFER",
			(bool (StateMachine<SipSMCommand,string>::*)(const SipSMCommand&)) &Call::handle_refer,
			s_start,
			s_inConference);

}

bool Call::start_inConference (const SipSMCommand &cmd) {

	/**
	* To avoid mcu from crash, check cmd contains any
	* commandPacket or just the String (Error message)
	*/	
	
	if ( cmd.getType()!=SipSMCommand::COMMAND_PACKET ) {
		return false;
	}else if(cmd.getCommandPacket()->getContentLength() >0 && cmd.getCommandPacket()->getContent()->getMemObjectType() != "SipMessageContentMime"){
		return false;//prajwol:: guys, we are not handling normal invite request here :)
	}
	
	//get the room of that call
	room = app->getRoom(app->readRoom(cmd.getCommandPacket()->getTo().getString()));

	if (transitionMatch("INVITE", cmd, SipSMCommand::transaction_layer, SipSMCommand::dialog_layer) && 
		room->isAuthorized(cmd.getCommandPacket()->getFrom().getUserIpString())) {
		
		//update tag
    	dialogState.updateState((SipRequest*)*cmd.getCommandPacket());
		
		//generate a 180 Ringing response
		MRef<SipMessage*> resp = new SipResponse(180,"Ringing",(SipRequest*)*cmd.getCommandPacket());
		MRef<SipHeaderValue *> contact = new SipHeaderValueContact(getDialogConfig()->getContactUri(false),-1);
		resp->addHeader( new SipHeader(*contact));
		resp->getHeaderValueTo()->setParameter("tag",dialogState.localTag);

		//send the response
		getSipStack()->enqueueCommand( SipSMCommand(resp,
				SipSMCommand::dialog_layer,
				SipSMCommand::transaction_layer));

		mimeInPacket = dynamic_cast <SipMessageContentMime*> (*(cmd.getCommandPacket())->getContent());
   		sdpInPacket = dynamic_cast <SdpPacket*> (*(mimeInPacket)->popFirstPart());
   		rclInPacket = dynamic_cast <SipMessageContentRCL*> (*(mimeInPacket)->popFirstPart());

		
		//Generate a 200 response	
		//update tag
    	dialogState.updateState((SipRequest*)*cmd.getCommandPacket());
		resp = new SipResponse(200,"ok",(SipRequest*)*cmd.getCommandPacket());
		
		resp->addHeader( new SipHeader(*contact)); 
		resp->getHeaderValueTo()->setParameter("tag",dialogState.localTag);
		resp->getHeaderValueTo()->setParameter("confID", "some_random_value");//prajwol:: add that unique THING for conversation
		resp->addHeader(new SipHeader(new SipHeaderValueUnknown("X-Conf",
							getDialogConfig()->sipIdentity->getSipUri().getString())));
		
		//add sdp
		MRef<SipMessageContentMime*> mimeContent = new SipMessageContentMime(
					mimeInPacket->getContentType(),
					"",
					mimeInPacket->getBoundry());
		mimeContent->addPart(dynamic_cast<SipMessageContent*> (*(room->getSdp()->getSdpPacket())));
		mimeContent->addPart(dynamic_cast<SipMessageContent*> (*rclInPacket));
		resp->setContent(dynamic_cast<SipMessageContent*> (*mimeContent));
		
		//send the response
		getSipStack()->enqueueCommand( SipSMCommand(resp, 
				SipSMCommand::dialog_layer, 
				SipSMCommand::transaction_layer));
		
		room->addParticipant(dialogState.callId, sdpInPacket);

		std::vector<std::string> participants = rclInPacket->getParticipantList();

   		std::vector <std::string>::const_iterator iter;
		iter = participants.begin();
		while(iter != participants.end()){
			//make call -->
			MRef<SipDialog*> callClient = new CallClient(getSipStack(), myIdentity, "" , app, dynamic_cast<SipMessageContent*> (*mimeContent));
			CommandString inv(callClient->getCallId(), SipCommandString::invite, *iter);
			SipSMCommand c(inv, SipSMCommand::dialog_layer, SipSMCommand::dialog_layer);
			getSipStack()->addDialog(callClient);
			callClient->handleCommand(c);
			//make call <--

			iter++;
		}
		return true;
	}
	return false;
}

bool Call::start_terminated_INVITE(const SipSMCommand &cmd) {
	
	if ( cmd.getType()!=SipSMCommand::COMMAND_PACKET ) {
		return false;
	}
	
	if (transitionMatch("INVITE", cmd, SipSMCommand::transaction_layer, SipSMCommand::dialog_layer)) {
		if(!room->isAuthorized(cmd.getCommandPacket()->getFrom().getUserIpString())) {
			
			this->dialogState.updateState((SipRequest*) *cmd.getCommandPacket());
			
			//send 401 Unauthorized
			MRef<SipMessage*> resp = new SipResponse( 401, 
								"Not authorized in that room", 
								(SipRequest*)*cmd.getCommandPacket() ); 
			resp->getHeaderValueTo()->setParameter("tag",dialogState.localTag);
			
			getSipStack()->enqueueCommand( SipSMCommand(resp, 
				SipSMCommand::dialog_layer, 
				SipSMCommand::transaction_layer));
			
			return true;
		}	
	}
	
	return false;
}

bool Call::inConference_waitingForByeResponse_BYEOut (const SipSMCommand &cmd) {
	
	if ( cmd.getType()!=SipSMCommand::COMMAND_STRING ) {
		return false;
	}
	
	//check the hangup action
	if (transitionMatch(cmd,
			"hangup",
			SipSMCommand::dialog_layer,
			SipSMCommand::dialog_layer ) ) {
		
		//send BYE
		MRef<SipRequest*> myBye = this->createSipMessageBye();
		SipSMCommand sipcmd( *myBye,
				SipSMCommand::dialog_layer,     // from layer (this class)
				SipSMCommand::transaction_layer // to layer (down the stack)
				);
		getSipStack()->enqueueCommand(sipcmd);
		return true;
	}
	return false;
}

bool Call::anyState_terminated (const SipSMCommand &cmd) {
	
	if ( cmd.getType()!=SipSMCommand::COMMAND_PACKET ) {
		return false;
	}
	
	if (transitionMatch("BYE",
				cmd,
				SipSMCommand::transaction_layer,
				SipSMCommand::dialog_layer)) {
		
		//update tag
		this->dialogState.updateState((SipRequest*) *cmd.getCommandPacket());
		
		//send 200 OK for the BYE msg
		MRef<SipMessage*> resp = new SipResponse( 200, "ok", (SipRequest*)*cmd.getCommandPacket() ); 
		resp->getHeaderValueTo()->setParameter("tag",dialogState.localTag);
		
		getSipStack()->enqueueCommand( SipSMCommand(resp, 
				SipSMCommand::dialog_layer, 
				SipSMCommand::transaction_layer));
		
		//terminate the call
		SipSMCommand sipcmd(
				CommandString (dialogState.callId,"call_terminated"),
				SipSMCommand::dialog_layer,     // from layer (this class)
				SipSMCommand::dispatcher        // to layer
				);
		getSipStack()->enqueueCommand(sipcmd);
		
		room->delParticipant(dialogState.callId);

		return true;
	}
	else if (transitionMatch("CANCEL", cmd,	SipSMCommand::transaction_layer, SipSMCommand::dialog_layer) ||
		transitionMatchSipResponse("*",cmd,SipSMCommand::transaction_layer, SipSMCommand::dialog_layer,"4**") ||
		transitionMatchSipResponse("*",cmd,SipSMCommand::transaction_layer, SipSMCommand::dialog_layer,"6**")) {
		
		//update tag
		this->dialogState.updateState((SipRequest*) *cmd.getCommandPacket());
		
		//terminate the call
		SipSMCommand sipcmd(
				CommandString (dialogState.callId,"call_terminated"),
				SipSMCommand::dialog_layer,     // from layer (this class)
				SipSMCommand::dispatcher        // to layer
				);
		getSipStack()->enqueueCommand(sipcmd);
		
		return true;		
	}
	return false;
}

bool Call::waitingForByeResponse_terminated_200 (const SipSMCommand &cmd) {
	
	if ( cmd.getType()!=SipSMCommand::COMMAND_PACKET ) {
		return false;
	}
	
	if (transitionMatchSipResponse("BYE", cmd,
			SipSMCommand::transaction_layer,
			SipSMCommand::dialog_layer,
			"2**")) {
		
		dialogState.updateState((SipResponse*)*cmd.getCommandPacket());
			
		//terminate the call
		SipSMCommand sipcmd(
				CommandString (dialogState.callId,"call_terminated"),
				SipSMCommand::dialog_layer,     // from layer (this class)
				SipSMCommand::dispatcher        // to layer
				);
		getSipStack()->enqueueCommand(sipcmd);
		
		room->delParticipant(dialogState.callId);
		
		return true;		
	}
	return false;
}

bool Call::anyState_terminated_cmd(const SipSMCommand &cmd) {
	if ( cmd.getType()!=SipSMCommand::COMMAND_STRING ) {
		return false;
	}
	
	//check the hangup action
	if (transitionMatch(cmd,
			"delete",
			SipSMCommand::dialog_layer,
			SipSMCommand::dialog_layer ) ) {
	
		//terminate the call
		SipSMCommand sipcmd(
				CommandString (dialogState.callId,"call_terminated"),
				SipSMCommand::dialog_layer,     // from layer (this class)
				SipSMCommand::dispatcher        // to layer
				);
		getSipStack()->enqueueCommand(sipcmd);
		
		room->delParticipant(dialogState.callId);
		return true;
	}
	return false;
	
}

string Call::getName(){
	return "Call";
}

string Call::getUri() {
	return getDialogConfig()->getContactUri(false).getString();
}

bool Call::handle_refer (const SipSMCommand &cmd) {

	if ( cmd.getType()!=SipSMCommand::COMMAND_PACKET ) {
			return false;
		}


	if (transitionMatch("REFER", cmd, SipSMCommand::transaction_layer, SipSMCommand::dialog_layer)) {

		dialogState.updateState((SipRequest*)*cmd.getCommandPacket());

		//Generate a 202 response

		MRef<SipMessage*> resp = new SipResponse(200,"Ok",(SipRequest*)*cmd.getCommandPacket());
		MRef<SipHeaderValue*> contact = new SipHeaderValueContact(getDialogConfig()->getContactUri(false),-1);
		resp->addHeader( new SipHeader(*contact));
		resp->getHeaderValueTo()->setParameter("tag",dialogState.localTag);
//		resp->addHeader(new SipHeader(new SipHeaderValueUnknown("X-Conf",
//				getDialogConfig()->sipIdentity->getSipUri().getString())));
		getSipStack()->enqueueCommand( SipSMCommand(resp,
						SipSMCommand::dialog_layer,
						SipSMCommand::transaction_layer));

//		send INVITE to sip uri in ReferTo
		MRef<SipMessageContentMime*> mimeInPacket ;// = dynamic_cast <SipMessageContentMime*> (*(cmd.getCommandPacket())->getContent());
		MRef<SipDialog*> callClient = new CallClient(getSipStack(), myIdentity, "" , app, dynamic_cast<SipMessageContent*> (*mimeInPacket));
		CommandString inv(callClient->getCallId(), SipCommandString::invite, cmd.getCommandPacket()->getHeaderValueNo(SIP_HEADER_TYPE_REFERTO,0)->getString());
		SipSMCommand c(inv, SipSMCommand::dialog_layer, SipSMCommand::dialog_layer);
		getSipStack()->addDialog(callClient);
		callClient->handleCommand(c);

//		send NOTIFY to the sender of REFER
		/* to be fixed */

		MRef<SipRequest*> req = new SipRequest("NOTIFY", dialogState.remoteUri) ; //getDialogConfig()->getContactUri());

		req->addDefaultHeaders(getDialogConfig()->sipIdentity->getSipUri(), dialogState.remoteUri,
				  dialogState.seqNo,dialogState.callId);
		req->addHeader(new SipHeader(new SipHeaderValueUserAgent(HEADER_USER_AGENT_DEFAULT)));
		req->addHeader(new SipHeader(new SipHeaderValueEvent("refer")));

		req->addHeader(new SipHeader(new SipHeaderValueContact(getDialogConfig()->sipIdentity->getContactUri(getSipStack(), false)))) ;


		req->addHeader(new SipHeader(new SipHeaderValueUnknown("Subscription-State", "active;expires=3600")));

		req->getHeaderValueTo()->setParameter("tag",dialogState.remoteTag);
		req->getHeaderValueFrom()->setParameter("tag",dialogState.localTag);

		MRef <SipMessageContent*> content = new SipMessageContentUnknown("SIP/2.0 100 Trying", "message/sipfrag;version=2.0") ;
		req->setContent(content) ;

		//send the response
		getSipStack()->enqueueCommand( SipSMCommand((MRef<SipMessage*>) *req,
				SipSMCommand::dialog_layer,
				SipSMCommand::transaction_layer));

		dialogState.updateState(req);
	}
}
