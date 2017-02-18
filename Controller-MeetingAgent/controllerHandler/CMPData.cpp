/*
 * CMPData.cpp
 *
 *  Created on: Feb 18, 2017
 *      Author: root
 */

#include "CMPData.h"



CMPData::CMPData()
{

}


CMPData::CMPData(int nFD,int nCommand,int nSequence, string bodyData)
{
	this->nFD = nFD;
	this->nCommand = nCommand;
	this->nSequence = nSequence;
	this->bodyData = string(bodyData);

}

CMPData::~CMPData()
{
}


bool CMPData::isVaild()
{
	if(nFD==-1 ||nCommand == -1|| nSequence == -1)
		return false;
	return true;

}

