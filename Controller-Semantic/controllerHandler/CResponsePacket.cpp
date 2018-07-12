/*
 * CResponsePacket.cpp
 *
 *  Created on: 2017年6月22日
 *      Author: Jugo
 */

#include "CResponsePacket.h"
#include "LogHandler.h"

#define DISPLAY				"display"
#define ACTIVITY			"activity"

using namespace std;

CResponsePacket::CResponsePacket() :
		jsonDisplay(0), jsonActivity(0)
{
	init();
}

CResponsePacket::~CResponsePacket()
{
	jsonDisplay->release();
	jsonActivity->release();

	delete jsonDisplay;
	delete jsonActivity;
}

void CResponsePacket::init()
{
	if(jsonDisplay && jsonDisplay->isValid())
	{
		jsonDisplay->release();
		delete jsonDisplay;
	}
	jsonDisplay = new JSONObject;
	jsonDisplay->create();

	if(jsonActivity && jsonActivity->isValid())
	{
		jsonActivity->release();
		delete jsonActivity;
	}
	jsonActivity = new JSONObject;
	jsonActivity->create();
}

void CResponsePacket::format(JSONObject &jResp)
{
	jResp.put(ACTIVITY, *jsonActivity);
	// 臉部表情不需要了 過渡期的垃圾
	//jResp.put(DISPLAY, *jsonDisplay);
}

void CResponsePacket::clear()
{
	init();
}

CResponsePacket &CResponsePacket::setDisplay(const char *szDisplayJSON)
{
	jsonDisplay->load(szDisplayJSON);
	return (*this);
}
