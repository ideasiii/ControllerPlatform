/*
 * CWeather.h
 *
 *  Created on: 2017年6月9日
 *      Author: Jugo
 */

#pragma once

typedef struct _WEATHER
{
	std::string strLocation;	// 位置
	std::string strWeather;		// 天氣
	float fTemperature;			// 平均溫度
	float fTemperature_min;		// 最低溫度
	float fTemperature_max;		// 最高溫度
	float fPressure;			// 氣壓
	float fVisibility;			// 能見度 公尺
	float fHumidity;			// 溼度
	long int lnToday;			// 今日日期 201706090000
	void clear()
	{
		strLocation.clear();
		strWeather.clear();
		fTemperature = 0.0;
		fTemperature_min = 0.0;
		fTemperature_max = 0.0;
		fPressure = 0.0;
		fVisibility = 0.0;
		fHumidity = 0.0;
		lnToday = 0;
	}
} WEATHER;

class CWeather
{
public:
	CWeather();
	virtual ~CWeather();
	void getWeather(const char *szLocation, WEATHER &weather);
};
