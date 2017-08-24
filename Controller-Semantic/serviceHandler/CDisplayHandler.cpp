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

#define DISPLAY_HOST			"https://smabuild.sytes.net/edubot/OCTOBO_Expressions/"
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

string CDisplayHandler::getDefaultDisplay()
{
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

	//=============== add json object to json array =============//
	jsonItem.put("host", DISPLAY_HOST);
	jsonItem.put("color", DISPLAY_COLOR);
	jsonItem.put("description", "default");
	for(int i = 0; i < 10; ++i)
	{
		jsonItem.put("time", 5 * i);
		jsonItem.put("file", format("OCTOBO_Expressions-%d.png", getRand(21, 40)));
		jsonArray.add(jsonItem);
	}

	//=============== set animation ================//
	jsonAnim.put("type", 5);
	jsonAnim.put("duration", 1000);
	jsonAnim.put("repeat", 1);
	jsonAnim.put("interpolate", 1);
	jsonItem.put("animation", jsonAnim);

	//================ add text json object to array item ==========//
	jsonText.put("type", 0);
	jsonItem.put("text", jsonText);

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
