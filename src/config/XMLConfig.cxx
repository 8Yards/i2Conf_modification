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

#include "XMLConfig.h"

using namespace std;

XMLConfig::XMLConfig(string file) {
	xmlfp = new XMLFileParser(file);
}

bool XMLConfig::readBool(string field, bool def) {
	bool ret = false;
	string a;
	
	string deft = "false";
	if(def) {
		deft = "true";
	}
	
	try {
		a = xmlfp->getValue(field, deft);
	}
	catch( XMLException &exc ){
		cout << "Error reading " << field << endl;
	}
	
	(a=="true") ? ret = true : ret = false;

	return ret;

}

int32_t XMLConfig::readInt(string field, int32_t def) {
	int32_t ret = -1;
	
	try{
		ret = xmlfp->getIntValue(field,def);
	}
	catch( XMLException &exc ){
		cout << "Error reading " << field << endl;
	}

	return ret;
}

string XMLConfig::readString(string field, string def) {
	string ret = "";
	
	try {
		ret = xmlfp->getValue(field, def);
	}
	catch( XMLException &exc ){
		cout << "Error reading " << field << endl;
	}

	return ret;
}
