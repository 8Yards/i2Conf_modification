/*
 * callClient.cpp
 *
 *  Created on: Nov 4, 2010
 *      Author: prajwol
 */

#include<string>

#include "callClient.h"

#include<libmsip/SipTransitionUtils.h>
#include<libmsip/SipHeaderMaxForwards.h>
#include<libmsip/SipHeaderCallID.h>
#include<libmsip/SipHeaderFrom.h>
#include<libmsip/SipHeaderTo.h>
#include<libmsip/SipHeaderCSeq.h>
#include<libmsip/SipHeaderContact.h>
#include<libmsip/SipHeaderUnknown.h>
#include<libmsip/SipCommandString.h>

using namespace std;

CallClient::CallClient(MRef<SipStack*> stack, MRef<SipIdentity*> ident,
		string cid, MRef<App*> app, MRef<SipMessageContent*> sipContent,
		string thread, string conversation) :
	SipDialog(stack, ident, cid), myIdentity(ident), app(app), mySipContent(
			sipContent) {

	State<SipSMCommand, string> *s_start = new State<SipSMCommand, string> (
			this, "start");
	addState(s_start);

	State<SipSMCommand, string> *s_calling = new State<SipSMCommand, string> (
			this, "calling");
	addState(s_calling);

	State<SipSMCommand, string> *s_incall = new State<SipSMCommand, string> (
			this, "incall");
	addState(s_incall);

	new StateTransition<SipSMCommand, string> (
			this,
			"transition_start_calling_INVITE",
			(bool(StateMachine<SipSMCommand, string>::*)(const SipSMCommand&)) &CallClient::start_calling_invite,
			s_start, s_calling);

	new StateTransition<SipSMCommand, string> (
			this,
			"transition_calling_incall_2xx",
			(bool(StateMachine<SipSMCommand, string>::*)(const SipSMCommand&)) &CallClient::calling_incall_2xx,
			s_calling, s_incall);

	new StateTransition<SipSMCommand, string> (
			this,
			"transition_incall_calling_REFER",
			(bool(StateMachine<SipSMCommand, string>::*)(const SipSMCommand&)) &CallClient::inCall_calling_refer,
			s_incall, s_calling);

	this->thread = thread;
	this->conversation = conversation;
}

bool CallClient::start_calling_invite(const SipSMCommand &command) {
	if (transitionMatch(command, SipCommandString::invite,
			SipSMCommand::dialog_layer, SipSMCommand::dialog_layer)) {
		++dialogState.seqNo;
		dialogState.remoteUri = command.getCommandString().getParam();

		myInvite = SipRequest::createSipMessageInvite(dialogState.callId,
				SipUri(dialogState.remoteUri),
				getDialogConfig()->sipIdentity->getSipUri(),
				getDialogConfig()->getContactUri(false), dialogState.seqNo,
				getSipStack());

		MRef<Room*> myRoom = app->getRoom(thread, conversation);
		myInvite->getHeaderValueFrom()->setParameter("tag",
				dialogState.localTag);
		myInvite->getHeaderValueTo()->setParameter(THREAD_ATTRIBUTE, thread);
		myInvite->getHeaderValueTo()->setParameter(CONVERSATION_ATTRIBUTE,
				conversation);

		myInvite->setContent(mySipContent);

		MRef<SipMessage*> pktr(*myInvite);
		SipSMCommand scmd(pktr, SipSMCommand::dialog_layer,
				SipSMCommand::transaction_layer);

		getSipStack()->enqueueCommand(scmd, HIGH_PRIO_QUEUE);

		return true;
	} else {
		return false;
	}
}

bool CallClient::inCall_calling_refer(const SipSMCommand &command) {
	return true;
}

bool CallClient::calling_incall_2xx(const SipSMCommand &command) {
	if (transitionMatchSipResponse("INVITE", command,
			SipSMCommand::transaction_layer, SipSMCommand::dialog_layer, "2**")) {

		MRef<Room*> myRoom = app->getRoom(thread, conversation);
		MRef<SipResponse*> resp((SipResponse*) *command.getCommandPacket());
		cout<< "callclient 2xx:: before "<< getCurrentStateName() <<endl;
		dialogState.updateState(resp);
		cout<< "callclient 2xx:: after "<< getCurrentStateName() <<endl;
		MRef<SipRequest*> ack = createSipMessageAck(myInvite);
		ack->setContent(
				dynamic_cast<SipMessageContent*> (*(myRoom->getSdp()->getSdpPacket())));

		SipSMCommand scmd(*ack, SipSMCommand::dialog_layer,
				SipSMCommand::transport_layer);
		getSipStack()->enqueueCommand(scmd, HIGH_PRIO_QUEUE);

		MRef<SipMessageContentMime*>
				mimeInPacket =
						dynamic_cast<SipMessageContentMime*> (*(command.getCommandPacket())->getContent());
		MRef<SdpPacket*> sdpPack =
				dynamic_cast<SdpPacket*> (*(mimeInPacket)->popFirstPart());

		myRoom->addParticipant(
				command.getCommandPacket()->getTo().getUserIpString(),
				dialogState.callId, sdpPack);

		return true;
	} else {
		return false;
	}
}

string CallClient::getName() {
	return "CallClient";
}

void CallClient::setMySipContent(MRef<SipMessageContent*> sipContent) {
	this->mySipContent = sipContent;
}

void CallClient::setConversationId(string id) {
	this->conversation = id;
}

MRef<SipRequest*> CallClient::getMyInvite() {
	return myInvite;
}

