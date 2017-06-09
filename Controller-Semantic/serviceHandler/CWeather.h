/*
 * CWeather.h
 *
 *  Created on: 2017年6月9日
 *      Author: Jugo
 */

#pragma once

typedef struct _WEATHER
{
	std::string strWeather;		// 天氣
	float fTemperature;			// 平均溫度
	float fTemperature_min;		// 最低溫度
	float fTemperature_max;		// 最高溫度
	float fPressure;			// 氣壓
	float fVisibility;			// 能見度 公尺
	float fHumidity;			// 溼度
	int nToday;					// 今日日期 20170609
} WEATHER;

class CWeather
{
public:
	CWeather();
	virtual ~CWeather();
	void getWeather(const char *szLocation, WEATHER &weather);
};
