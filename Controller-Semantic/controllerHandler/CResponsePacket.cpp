/*
 * CResponsePacket.cpp
 *
 *  Created on: 2017年6月22日
 *      Author: Jugo
 */

#include "CResponsePacket.h"
#include "config.h"
#include "LogHandler.h"
#include "utility.h"
//#include "JSONArray.h"
//#include "JSONObject.h"
#include "common.h"
#include <map>

#define DISPLAY				"display"
#define ACTIVITY			"activity"
#define SHOW				"show"
#define ANIMATION			"animation"
#define TEXT				"text"

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
	jResp.put(DISPLAY, *jsonDisplay);
}

void CResponsePacket::clear()
{
	init();
}

/*
template<typename T>
CResponsePacket &CResponsePacket::setActivity(const char *szKey, T tValue)
{
	jsonActivity->put(szKey, tValue);
	return (*this);
}
*/
