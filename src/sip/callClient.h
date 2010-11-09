/*
 * callClient.h
 *
 *  Created on: Nov 4, 2010
 *      Author: prajwol
 */

#ifndef CALLCLIENT_H_
#define CALLCLIENT_H_

#include<libmsip/SipStack.h>
#include<libmsip/SipDialog.h>

class App;
using namespace std;

class CallClient: public SipDialog{
public:
	CallClient(MRef<SipStack*> stack, MRef<SipIdentity*> ident, string cid, MRef<App*> app);

	string getName();

	bool start_calling_invite(const SipSMCommand &command);
	bool calling_incall_2xx(const SipSMCommand &command);

private:
	MRef<App*> app;
	MRef<SipIdentity*> myIdentity;
};
#include "../app.h"
#endif /* CALLCLIENT_H_ */
