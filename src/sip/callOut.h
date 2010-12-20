/*
 * callClient.h
 *
 *  Created on: Nov 4, 2010
 *      Author: prajwol, nina
 */

#ifndef CALLOUT_H_
#define CALLOUT_H_

#include<libmsip/SipStack.h>
#include<libmsip/SipDialog.h>

class App;
using namespace std;

class CallOut: public SipDialog{
public:
	CallOut(MRef<SipStack*> stack, MRef<SipIdentity*> ident, string cid, MRef<App*> app, MRef<SipMessageContent*> sipContent,
			string thread, string conversation);

	string getName();

	bool start_calling_invite(const SipSMCommand &command);
	bool calling_incall_2xx(const SipSMCommand &command);
	bool inCall_calling_refer(const SipSMCommand &command);

	void setMySipContent(MRef<SipMessageContent*> sipContent);
	void setConversationId(string id);
	MRef<SipRequest*> getMyInvite();

private:
	MRef<App*> app;
	MRef<SipRequest*> myInvite;
	MRef<SipIdentity*> myIdentity;
	MRef<SipMessageContent*> mySipContent;
	string thread ;
	string conversation ;
};
#include "../app.h"
#endif /* CALLCLIENT_H_ */
