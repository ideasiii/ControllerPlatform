/*
 * CMPData.h
 *
 *  Created on: Feb 18, 2017
 *      Author: root
 */

#include <string>

#ifndef CONTROLLER_MEETINGAGENT_CONTROLLERHANDLER_CMPDATA_H_
#define CONTROLLER_MEETINGAGENT_CONTROLLERHANDLER_CMPDATA_H_

using namespace std;

class CMPData
{
public:
	CMPData();
	CMPData(int,int,int ,string);
	virtual ~CMPData();
	int nFD;
	int nSequence;
	int nCommand;
	string bodyData;
	bool isVaild();

};

#endif /* CONTROLLER_MEETINGAGENT_CONTROLLERHANDLER_CMPDATA_H_ */
