/*
 * CMPData.h
 *
 *  Created on: Feb 18, 2017
 *      Author: root
 */

#include <string>

#ifndef CONTROLLER_MEETINGAGENT_CONTROLLERHANDLER_CMPDATA_H_
#define CONTROLLER_MEETINGAGENT_CONTROLLERHANDLER_CMPDATA_H_

class CMPData
{
public:
	CMPData();
	CMPData(CMPData *);
	CMPData(int, int, int, std::string);
	virtual ~CMPData();
	int nFD;
	int nSequence;
	int nCommand;
	std::string bodyData;
	bool isVaild();

};

#endif /* CONTROLLER_MEETINGAGENT_CONTROLLERHANDLER_CMPDATA_H_ */
