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

#include<libmnetutil/init.h>
#include<libmcrypto/init.h>
#include<libmutil/Thread.h>

#include<string>

#include "app.h"


using namespace std;
using namespace log4cxx;
using namespace log4cxx::helpers;

static LoggerPtr logger(Logger::getLogger("strManager"));

int port;

void usage(string progname){
	cerr << "Usage: "<< progname <<" -f <config_file>" <<endl;
	exit(1);
}

int ua_main(int argc, char **argv){

	string configFile;
	
	for (int i = 1; i < argc; i += 2){
		if (string("-f")==argv[i]) {
			configFile=argv[i+1];
		}
		
		if(string("-debug")==argv[i]) { 
			mdbg.setEnabled(true);
		}
		else {
			mdbg.setEnabled(false);
		}
	}

	MRef<App*> app = new App(configFile);

	Thread t(*app);

	while (true){
		string line;
		getline(cin,line);
		if (!cin){
			return 0;
		}
		line = trim(line);
		if (line==""){
			cout << "<------------- Sip Stack Status ------------->" << endl;
			cout << app->sipStack->getStackStatusDebugString();
			cout << "<-------------------------------------------->" << endl;
		}else{
			if (line.substr(0,6)=="hangup"){
				string callId = trim(line.substr(6));
				if (callId.size()==0)
					callId = app->getLastCallId();
				
				CommandString cmd(callId, "hangup");
				app->sipStack->handleCommand(cmd);
			}
			else if (line.substr(0,6)=="printm"){
				cout << app->getStreamManager()->printManagement() << endl;
			}
			else if (line.substr(0,3)=="del"){
				string callId = trim(line.substr(3));
				
				CommandString cmd(callId, "delete");
				app->sipStack->handleCommand(cmd);			
			}
			else if (line.substr(0,4)=="exit"){
				exit(0);
			}		
		}
	}
	return 0;
}

int main(int argc, char **argv){
	libmnetutilInit();
	libmcryptoInit();
	srand(time(0));
	
	// PropertyConfigurator con la ruta a las properties.
    	PropertyConfigurator::configure("conf/logger.properties");
	
	return ua_main(argc, argv);
}
