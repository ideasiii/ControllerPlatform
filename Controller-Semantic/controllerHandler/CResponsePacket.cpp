/*
 * CResponsePacket.cpp
 *
 *  Created on: 2017年6月22日
 *      Author: Jugo
 */

#include "CResponsePacket.h"
#include "JSONObject.h"
#include "config.h"
#include "LogHandler.h"

template<typename T>
CResponsePacket<T>::CResponsePacket()
{

}

template<typename T>
CResponsePacket<T>::~CResponsePacket()
{

}

template<typename T>
void CResponsePacket<T>::format(int nType, T &data, JSONObject &jResp)
{
	switch(data.type)
	{
	case TYPE_RESP_UNKNOW:
		break;
	case TYPE_RESP_MUSIC_SPOTIFY:
		_log("[CResponsePacket] format type: TYPE_RESP_SPOTIFY");
		break;
	case TYPE_RESP_STORY:
		_log("[CResponsePacket] format type: TYPE_RESP_STORY");
		break;
	case TYPE_RESP_TTS:
		break;
	case TYPE_RESP_MUSIC_LOCAL:
		break;
	}
}

