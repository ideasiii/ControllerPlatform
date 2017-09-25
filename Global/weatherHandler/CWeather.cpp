/*
 * CWeather.cpp
 *
 *  Created on: 2017年6月9日
 *      Author: Jugo
 */

#include <set>
#include <string>
#include "CWeather.h"
#include "JSONObject.h"
#include "JSONArray.h"
#include "CHttpsClient.h"
#include "utility.h"
#include "LogHandler.h"

#define WEATHER_CURRENT			"http://api.openweathermap.org/data/2.5/weather?q=%s,tw&appid=63daed3b782efe4bef4fbe300ea8acab&lang=zh_tw"

using namespace std;

CWeather::CWeather()
{

}

CWeather::~CWeather()
{

}

void CWeather::getWeather(const char *szLocation, WEATHER &weather)
{
	string strURL;
	string strData;
	CHttpsClient httpsClient;
	set<string> setHead;
	JSONObject jroot;
	JSONArray jArray;
	JSONObject jItem;
	int nYear, nMonth, nDay, nHour, nMin, nSec;

	weather.strLocation.clear();
	if(!szLocation)
		return;

	strURL = format(WEATHER_CURRENT, urlEncode(szLocation).c_str());
	httpsClient.GET(strURL.c_str(), strData, setHead);

	if(!strData.empty())
	{
		jroot.load(strData);

		if(jroot.isValid())
		{
			jArray = jroot.getJsonArray("weather");
			if(jArray.isValid())
			{
				jItem = jArray.getJsonObject(0);
				if(jItem.isValid())
				{
					weather.strWeather = jItem.getString("description");
					if(!weather.strWeather.empty() && !weather.strWeather.compare("多雲"))
					{
						weather.strWeather = "多雲時晴";
					}
				}
			}

			jItem = jroot.getJsonObject("main");
			if(jItem.isValid())
			{
				weather.fTemperature = jItem.getFloat("temp") - 273.15; // ℃ = K - 273.15
				weather.fPressure = jItem.getFloat("pressure");
				weather.fHumidity = jItem.getFloat("humidity");
				weather.fTemperature_max = jItem.getFloat("temp_max") - 273.15;
				weather.strLocation = szLocation;
			}
			weather.fVisibility = jroot.getInt("visibility");
			currentDateTimeNum(nYear, nMonth, nDay, nHour, nMin, nSec);
			weather.lnToday = nowSecond();
		}
		jroot.release();
	}
}

