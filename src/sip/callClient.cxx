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

CallClient::CallClient(MRef<SipStack*> stack, MRef<SipIdentity*> ident, string cid, MRef<App*> app):
	SipDialog(stack, ident, cid), myIdentity(ident), app(app) {

	State<SipSMCommand, string> *s_start = new State<SipSMCommand, string> (this, "start");
	addState(s_start);

	State<SipSMCommand, string> *s_calling = new State<SipSMCommand, string> (this, "calling");
	addState(s_calling);

	State<SipSMCommand, string> *s_incall = new State<SipSMCommand, string> (this, "incall");
		addState(s_calling);

	new StateTransition<SipSMCommand, string> (
			this,
			"transition_start_calling_invite",
			(bool(StateMachine<SipSMCommand, string>::*)(const SipSMCommand&)) &CallClient::start_calling_invite,
			s_start, s_calling);

	new StateTransition<SipSMCommand, string> (
			this,
			"transition_calling_incall_2xx",
			(bool(StateMachine<SipSMCommand, string>::*)(const SipSMCommand&)) &CallClient::calling_incall_2xx,
			s_calling, s_incall);
}

bool CallClient::start_calling_invite(const SipSMCommand &command){
	if (transitionMatch(command, SipCommandString::invite, SipSMCommand::dialog_layer, SipSMCommand::dialog_layer)){
		++dialogState.seqNo;
		dialogState.remoteUri= command.getCommandString().getParam();

		//--sendInvite();
		myInvite = SipRequest::createSipMessageInvite(
				dialogState.callId,
				SipUri(dialogState.remoteUri),
				getDialogConfig()->sipIdentity->getSipUri(),
				getDialogConfig()->getContactUri(false),
				dialogState.seqNo,
				getSipStack() ) ;

		MRef<Room*> myRoom = app->getRoom("default");
		myInvite->getHeaderValueFrom()->setParameter("tag",dialogState.localTag );
		myInvite->setContent(dynamic_cast<SipMessageContent*> (*(myRoom->getSdp()->getSdpPacket())));

		MRef<SipMessage*> pktr(*myInvite);
		SipSMCommand scmd(
				pktr,
				SipSMCommand::dialog_layer,
				SipSMCommand::transaction_layer
				);

		getSipStack()->enqueueCommand(scmd, HIGH_PRIO_QUEUE);
		//--

		return true;
	}else{
		return false;
	}
}

bool CallClient::calling_incall_2xx(const SipSMCommand &command){
	if (transitionMatchSipResponse("INVITE", command, SipSMCommand::transaction_layer, SipSMCommand::dialog_layer, "2**")){
		MRef<SipResponse*> resp(  (SipResponse*)*command.getCommandPacket() );
		dialogState.updateState( resp );
		MRef<Room*> myRoom = app->getRoom("default");

		//--sendAck();
		MRef<SipRequest*> ack = createSipMessageAck(myInvite);
		ack->setContent(dynamic_cast<SipMessageContent*> (*(myRoom->getSdp()->getSdpPacket())));

		SipSMCommand scmd(
				*ack,
				SipSMCommand::dialog_layer,
				SipSMCommand::transport_layer
				);

		getSipStack()->enqueueCommand(scmd, HIGH_PRIO_QUEUE);

		MRef<SdpPacket*> sdpPack = dynamic_cast <SdpPacket*> (*(command.getCommandPacket())->getContent());
		myRoom->addParticipant(dialogState.callId, sdpPack);

		return true;
	}else{
		return false;
	}
}

string CallClient::getName(){
	return "CallClient";
}
