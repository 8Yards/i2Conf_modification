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

using namespace std;

Call::Call(MRef<SipStack*> stack, MRef<SipIdentity*> ident, string cid, MRef<
		App*> app) :
	SipDialog(stack, ident, cid), myIdentity(ident), app(app) {

	//start state
	State<SipSMCommand, string> *s_start = new State<SipSMCommand, string> (
			this, "start");
	addState(s_start);

	//calling state
	State<SipSMCommand, string> *s_calling = new State<SipSMCommand, string> (
			this, "calling");
	addState(s_calling);

	//inCall state
	State<SipSMCommand, string> *s_inCall = new State<SipSMCommand, string> (
			this, "inCall");
	addState(s_inCall);

	//terminated state
	State<SipSMCommand, string> *s_terminated = new State<SipSMCommand, string> (
			this, "terminated");
	addState(s_terminated);

	//from start to inCall - for incoming calls
	new StateTransition<SipSMCommand, string> (
			this,
			"transition_start_inCall_inviteIn",
			(bool(StateMachine<SipSMCommand, string>::*)(const SipSMCommand&)) &Call::start_inCall_inviteIn,
			s_start, s_inCall);

	//from start to calling - for outgoing calls
	new StateTransition<SipSMCommand, string> (
			this,
			"transition_start_calling_inviteOut",
			(bool(StateMachine<SipSMCommand, string>::*)(const SipSMCommand&)) &Call::start_calling_inviteOut,
			s_start, s_calling);

	//from calling to inCall - for incoming 200
	new StateTransition<SipSMCommand, string> (
			this,
			"transition_calling_inCall_200",
			(bool(StateMachine<SipSMCommand, string>::*)(const SipSMCommand&)) &Call::calling_inCall_200,
			s_calling, s_inCall);

	//from inCall to inCall - for incoming refer
	new StateTransition<SipSMCommand, string> (
			this,
			"transition_inCall_inCall_refer",
			(bool(StateMachine<SipSMCommand, string>::*)(const SipSMCommand&)) &Call::inCall_inCall_refer,
			s_inCall, s_inCall);

	//from inCall to calling - for reinvites
	new StateTransition<SipSMCommand, string> (
			this,
			"transition_inCall_calling_invite",
			(bool(StateMachine<SipSMCommand, string>::*)(const SipSMCommand&)) &Call::inCall_calling_invite,
			s_inCall, s_calling);

	//from inCall to terminated - for incoming bye
	new StateTransition<SipSMCommand, string> (
			this,
			"transition_inCall_terminated",
			(bool(StateMachine<SipSMCommand, string>::*)(const SipSMCommand&)) &Call::inCall_terminated_bye,
			s_inCall, s_terminated);

	//from any state to terminated - for incoming CANCEL or 4** status to any request
	new StateTransition<SipSMCommand, string>(
			this,
			"transition_anyState_terminated",
			(bool(StateMachine<SipSMCommand, string>::*)(const SipSMCommand&)) &Call::anyState_terminated,
			anyState, s_terminated);

	//from any state to terminated - hangup from command line
	new StateTransition<SipSMCommand, string>(
				this,
				"transition_anyState_terminated",
				(bool(StateMachine<SipSMCommand, string>::*)(const SipSMCommand&)) &Call::anyState_terminated_hangup_cmd,
				anyState, s_terminated);
}

bool Call::start_inCall_inviteIn(const SipSMCommand &cmd) {
	if (!transitionMatch("INVITE", cmd, SipSMCommand::transaction_layer,
			SipSMCommand::dialog_layer)) {
		return false;
	}

	MRef<SipMessage*> commandPacket = cmd.getCommandPacket();

	if (commandPacket->getContentLength() > 0
			&& commandPacket->getContent()->getMemObjectType()
					!= "SipMessageContentMime") {
		//TODO:: send reject
		return false;
	}

	MRef<SipMessageContentMime*> mimeInPacket =
			dynamic_cast<SipMessageContentMime*> (*(commandPacket->getContent()));
	MRef<SdpPacket*> sdpInPacket =
			dynamic_cast<SdpPacket*> (*(mimeInPacket)->popFirstPart());
	MRef<SipMessageContentRCL*> rclInPacket =
			dynamic_cast<SipMessageContentRCL*> (*(mimeInPacket)->popFirstPart());

	room = app->getRoom(
			commandPacket->getHeaderValueTo()->getParameter(THREAD_ATTRIBUTE),
			commandPacket->getHeaderValueTo()->getParameter(CONVERSATION_ATTRIBUTE),
			true);
	room->getMimePacket()->replacePart(*rclInPacket);

	dialogState.updateState((SipRequest*) *commandPacket);
	dialogState.remoteUri = commandPacket->getFrom().getUserIpString();

	MRef<SipMessage*> resp = new SipResponse(200, "OK", (SipRequest*) *commandPacket);
	MRef<SipHeaderValue *> contact =
			new SipHeaderValueContact(getDialogConfig()->getContactUri(false), -1);
	resp->addHeader(new SipHeader(*contact));
	resp->getHeaderValueTo()->setParameter("tag", dialogState.localTag);
	resp->setContent(*room->getMimePacket());
	getSipStack()->enqueueCommand(SipSMCommand(resp,
			SipSMCommand::dialog_layer, SipSMCommand::transaction_layer));

	room->authorize(dialogState.remoteUri);
	room->addParticipant(dialogState.remoteUri, dialogState.callId, sdpInPacket);

	vector<string> rclList = rclInPacket->getParticipantList();
	for (vector<string>::const_iterator iterRclList = rclList.begin(); iterRclList != rclList.end(); iterRclList++) {
		if (*iterRclList == dialogState.remoteUri) {
			//Dont call the same user again
			continue;
		}

		MRef<Call*> call = new Call(getSipStack(), myIdentity, "", app);
		call->setRoom(room);
		CommandString inv(call->getCallId(), SipCommandString::invite, *iterRclList);
		SipSMCommand c(inv, SipSMCommand::dialog_layer, SipSMCommand::dialog_layer);
		getSipStack()->addDialog(dynamic_cast<SipDialog*> (*call));
		call->handleCommand(c);

		room->authorize(*iterRclList);
		app->addCall(call);
	}
	return true;
}

bool Call::start_calling_inviteOut(const SipSMCommand &cmd) {
	return callOut(cmd, false);
}

bool Call::calling_inCall_200(const SipSMCommand &cmd){
	if (!transitionMatchSipResponse("INVITE", cmd,
				SipSMCommand::transaction_layer, SipSMCommand::dialog_layer, "2**")) {
		return false;
	}

	dialogState.updateState((SipResponse*) *cmd.getCommandPacket());
	MRef<SipRequest*> ack = createSipMessageAck(lastInvite);
	SipSMCommand scmd(*ack, SipSMCommand::dialog_layer,	SipSMCommand::transport_layer);
	getSipStack()->enqueueCommand(scmd, HIGH_PRIO_QUEUE);

	MRef<SipMessage*> commandPacket = cmd.getCommandPacket();

	//only add participant if it has MIME, this caters OK to reinvites
	if (commandPacket->getContentLength() > 0) {
		if (commandPacket->getContent()->getMemObjectType()
				== "SipMessageContentMime") {
			MRef<SipMessageContentMime*>
					mimeInPacket =
							dynamic_cast<SipMessageContentMime*> (*(commandPacket->getContent()));
			MRef<SdpPacket*> sdpInPacket =
					dynamic_cast<SdpPacket*> (*(mimeInPacket)->popFirstPart());

			room->addParticipant(commandPacket->getTo().getUserIpString(),
					dialogState.callId, sdpInPacket);
		}
	}

	return true;
}

bool Call::inCall_inCall_refer(const SipSMCommand &cmd){
	if (!transitionMatch("REFER", cmd, SipSMCommand::transaction_layer,
			SipSMCommand::dialog_layer)) {
		return false;
	}

	dialogState.updateState((SipRequest*) *cmd.getCommandPacket());
	MRef<SipMessage*> resp = new SipResponse(200, "OK",	(SipRequest*) *cmd.getCommandPacket());
	MRef<SipHeaderValue*> contact =
			new SipHeaderValueContact(getDialogConfig()->getContactUri(false), -1);
	resp->addHeader(new SipHeader(*contact));
	resp->getHeaderValueTo()->setParameter("tag", dialogState.localTag);
	getSipStack()->enqueueCommand(SipSMCommand(resp,
			SipSMCommand::dialog_layer, SipSMCommand::transaction_layer));

	//send INVITE to sip uri in ReferTo
	MRef<SipUri*> referTo =
			new SipUri(cmd.getCommandPacket()->getHeaderValueNo(
					SIP_HEADER_TYPE_REFERTO, 0)->getString());

	string conversation =
			cmd.getCommandPacket()->getHeaderValueTo()->getParameter(
					CONVERSATION_ATTRIBUTE);

	room = app->replaceRoom(room->getThreadId(), room->getConversationId(),
			conversation);

	MRef<SipMessageContentMime*> mimeContent = room->getMimePacket();
	MRef<SipMessageContentRCL*> rclInPacket = new SipMessageContentRCL(
			room->getRcl() + "," + referTo->getUserIpString(),
			"application/resource-lists+xml");
	mimeContent->replacePart(dynamic_cast<SipMessageContent*> (*rclInPacket));

	MRef<Call*> call = new Call(getSipStack(), myIdentity, "", app);
	call->setRoom(room);
	CommandString inv(call->getCallId(), SipCommandString::invite, referTo->getUserIpString());
	SipSMCommand c(inv, SipSMCommand::dialog_layer, SipSMCommand::dialog_layer);
	getSipStack()->addDialog(dynamic_cast<SipDialog*> (*call));
	app->addCall(call);
	call->handleCommand(c);

	// send updated RCL to all clients in the room
	map<string, MRef<Participant*> > partList = room->getParticipants();
	for (map<string, MRef<Participant*> >::iterator iterPartList = partList.begin(); iterPartList != partList.end(); iterPartList++) {
		if (iterPartList->second->getCallId() == dialogState.callId) {
			//Dont call the same user again
			continue;
		}

		MRef<Call*> referCall = app->getCallById(iterPartList->second->getCallId());
		if (referCall.isNull()) {
			cout << "I didn't find dialog # " << iterPartList->second->getCallId() << endl;
			continue;
		}

		CommandString inv("", SipCommandString::invite, "");
		SipSMCommand c(inv, SipSMCommand::dialog_layer,	SipSMCommand::dialog_layer);
		referCall->handleCommand(c);
	}

	return true;
}

	//		send NOTIFY to the sender of REFER
	/* to be fixed */

	//		MRef<SipRequest*> req = new SipRequest("NOTIFY", dialogState.remoteUri); //getDialogConfig()->getContactUri());
	//
	//		req->addDefaultHeaders(getDialogConfig()->sipIdentity->getSipUri(),
	//				dialogState.remoteUri, dialogState.seqNo, dialogState.callId);
	//		req->addHeader(new SipHeader(new SipHeaderValueUserAgent(
	//				HEADER_USER_AGENT_DEFAULT)));
	//		req->addHeader(new SipHeader(new SipHeaderValueEvent("refer")));
	//		req->addHeader(new SipHeader(new SipHeaderValueContact(
	//				getDialogConfig()->sipIdentity->getContactUri(getSipStack(),
	//						false))));
	//		req->addHeader(new SipHeader(new SipHeaderValueUnknown(
	//				"Subscription-State", "active;expires=3600")));
	//		req->getHeaderValueTo()->setParameter("tag", dialogState.remoteTag);
	//		req->getHeaderValueFrom()->setParameter("tag", dialogState.localTag);
	//
	//		MRef<SipMessageContent*> content = new SipMessageContentUnknown(
	//				"SIP/2.0 100 Trying", "message/sipfrag;version=2.0");
	//		req->setContent(content);

	//send notify request
	//		getSipStack()->enqueueCommand( SipSMCommand((MRef<SipMessage*>) *req,
	//				SipSMCommand::dialog_layer,
	//				SipSMCommand::transaction_layer));
	//
	//		dialogState.updateState(req);

bool Call::inCall_calling_invite(const SipSMCommand &cmd){
	return callOut(cmd, true);
}

bool Call::inCall_terminated_bye(const SipSMCommand &cmd) {
	if (!transitionMatch("BYE", cmd, SipSMCommand::transaction_layer,
			SipSMCommand::dialog_layer)) {
		return false;
	}

	//send 200 OK for the BYE msg
	MRef<SipMessage*> resp = new SipResponse(200, "ok",	(SipRequest*) *cmd.getCommandPacket());
	resp->getHeaderValueTo()->setParameter("tag", dialogState.localTag);
	getSipStack()->enqueueCommand(SipSMCommand(resp, SipSMCommand::dialog_layer, SipSMCommand::transaction_layer));

	releaseResources();
	return true;
}

bool Call::anyState_terminated(const SipSMCommand &cmd) {
	if(!transitionMatch("CANCEL", cmd, SipSMCommand::transaction_layer, SipSMCommand::dialog_layer) &&
			!transitionMatchSipResponse("INVITE", cmd, SipSMCommand::transaction_layer, SipSMCommand::dialog_layer, "4**")) {
		return false;
	}

	dialogState.updateState((SipRequest*) *cmd.getCommandPacket()) ;

	releaseResources();
	return true;
}

bool Call::anyState_terminated_hangup_cmd(const SipSMCommand &cmd) {
	if (!transitionMatch(cmd, "hangup", SipSMCommand::dialog_layer, SipSMCommand::dialog_layer)) {
		return false;
	}

	//send bye
	MRef<SipMessage*> bye = (SipMessage*)(*createSipMessageBye());
    getSipStack()->enqueueCommand(SipSMCommand(bye, SipSMCommand::dialog_layer, SipSMCommand::transaction_layer));

    releaseResources();
	return true ;
}

bool Call::callOut(const SipSMCommand &cmd, bool isReInvite) {
	if (!transitionMatch(cmd, SipCommandString::invite,
			SipSMCommand::dialog_layer, SipSMCommand::dialog_layer)) {
		return false;
	}
	++dialogState.seqNo;
	if (!isReInvite) {
		dialogState.remoteUri = cmd.getCommandString().getParam();
	}

	lastInvite = SipRequest::createSipMessageInvite(
			dialogState.callId, SipUri(dialogState.remoteUri),
			getDialogConfig()->sipIdentity->getSipUri(),
			getDialogConfig()->getContactUri(false), dialogState.seqNo,
			getSipStack());

	lastInvite->getHeaderValueFrom()->setParameter("tag", dialogState.localTag);
	lastInvite->getHeaderValueTo()->setParameter(THREAD_ATTRIBUTE, room->getThreadId());
	lastInvite->getHeaderValueTo()->setParameter(CONVERSATION_ATTRIBUTE, room->getConversationId());
	lastInvite->setContent(*room->getMimePacket());

	MRef<SipMessage*> pktr(*lastInvite);
	SipSMCommand scmd(pktr, SipSMCommand::dialog_layer,	SipSMCommand::transaction_layer);
	getSipStack()->enqueueCommand(scmd, HIGH_PRIO_QUEUE);

	return true;
}

void Call::releaseResources()
{
    SipSMCommand sipcmd(CommandString(dialogState.callId, "call_terminated"), SipSMCommand::dialog_layer, SipSMCommand::dispatcher);
    room->delParticipant(dialogState.callId);
    app->removeCallId(dialogState.callId);
    getSipStack()->enqueueCommand(sipcmd);
}

string Call::getName() {
	return "Call";
}

string Call::getUri() {
	return getDialogConfig()->getContactUri(false).getString();
}

MRef<Room*> Call::getRoom() {
	return room;
}

void Call::setRoom(MRef<Room*> room) {
	this->room = room;
}
