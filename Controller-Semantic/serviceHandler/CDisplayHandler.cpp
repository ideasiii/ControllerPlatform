/*
 * CDisplayHandler.cpp
 *
 *  Created on: 2017年8月24日
 *      Author: Jugo
 */

#include "CDisplayHandler.h"
#include "JSONObject.h"
#include "JSONArray.h"
#include "utility.h"

#define DISPLAY_HOST			"https://ryejuice.sytes.net/edubot/OCTOBO_Expressions/"
#define DISPLAY_COLOR			"#FFA0C9EC"

using namespace std;

CDisplayHandler::CDisplayHandler()
{
	mapDisplayFile[0] = "OCTOBO_Expressions-40.png";
	mapDisplayFile[1] = "OCTOBO_Expressions-37.png";
	mapDisplayFile[2] = "OCTOBO_Expressions-35.png";
	mapDisplayFile[3] = "OCTOBO_Expressions-40.png";
	mapDisplayFile[4] = "OCTOBO_Expressions-40.png";
	mapDisplayFile[5] = "OCTOBO_Expressions-40.png";
	mapDisplayFile[6] = "OCTOBO_Expressions-40.png";
	mapDisplayFile[7] = "OCTOBO_Expressions-40.png";
	mapDisplayFile[8] = "OCTOBO_Expressions-40.png";
	mapDisplayFile[9] = "OCTOBO_Expressions-40.png";
}

CDisplayHandler::~CDisplayHandler()
{

}

string CDisplayHandler::getDisplay(int nDisplay)
{
	int i;
	string strJSON;
	JSONObject jsonRoot;
	JSONArray jsonArray;
	JSONObject jsonItem;
	JSONObject jsonAnim;
	JSONObject jsonText;

	jsonRoot.create();
	jsonArray.create();
	jsonItem.create();
	jsonAnim.create();
	jsonText.create();

	jsonRoot.put("enable", 1);

	//=============== set json item object =============//
	jsonItem.put("host", DISPLAY_HOST);
	jsonItem.put("color", DISPLAY_COLOR);

	//=============== add animation json object to item ================//
	jsonAnim.put("type", 5);
	jsonAnim.put("duration", 1000);
	jsonAnim.put("repeat", 1);
	jsonAnim.put("interpolate", 1);
	jsonItem.put("animation", jsonAnim);

	//================ add text json object to item ==========//
	jsonText.put("type", 0);
	jsonItem.put("text", jsonText);

	switch(nDisplay)
	{
	case DEFAULT:
		jsonItem.put("description", "default");
		//================ add json item object to array ==========//
		for(i = 0; i < 10; ++i)
		{
			jsonItem.put("time", 3 * i);
			jsonItem.put("file", format("OCTOBO_Expressions-%d.png", getRand(21, 40)));
			jsonArray.add(jsonItem);
		}
		break;
	case SAD:
		jsonItem.put("description", "sad");
		for(i = 0; i < 5; ++i)
		{
			if(i % 2)
			{
				jsonItem.put("time", 3 * i);
				jsonItem.put("file", "OCTOBO_Expressions-05.png");
				jsonArray.add(jsonItem);
			}
			else
			{
				jsonItem.put("time", 3 * i);
				jsonItem.put("file", "OCTOBO_Expressions-06.png");
				jsonArray.add(jsonItem);
			}
		}
		break;
	case HAPPY:
		jsonItem.put("description", "happy");
		for(i = 0; i < 5; ++i)
		{
			if(i % 2)
			{
				jsonItem.put("time", 3 * i);
				jsonItem.put("file", "OCTOBO_Expressions-31.png");
				jsonArray.add(jsonItem);
			}
			else
			{
				jsonItem.put("time", 3 * i);
				jsonItem.put("file", "OCTOBO_Expressions-35.png");
				jsonArray.add(jsonItem);
			}
		}
		break;
	default:
		jsonItem.put("time", 0);
		jsonItem.put("file", "OCTOBO_Expressions-14.png");
		jsonArray.add(jsonItem);
		break;
	}

	//============== add jsonarray to json object =========//
	jsonRoot.put("show", jsonArray);

	strJSON = jsonRoot.toJSON();

	jsonText.release();
	jsonAnim.release();
	jsonItem.release();
	jsonArray.release();
	jsonRoot.release();

	return strJSON;
}

string CDisplayHandler::getDefaultDisplay()
{
	return getDisplay(DEFAULT);
}

string CDisplayHandler::getSadDisplay()
{
	return getDisplay(SAD);
}

string CDisplayHandler::getHappyDisplay()
{
	return getDisplay(HAPPY);
}
