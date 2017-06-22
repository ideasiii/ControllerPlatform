/*
 * CResponsePacket.h
 *
 *  Created on: 2017年6月22日
 *      Author: Jugo
 */

#pragma once

#include <map>
#include <string>
using namespace std;

class JSONObject;

typedef struct _RSP_MUSIC
{
	int type;				//	Response type: 1：Spotify音樂 , 4：Local音樂
	int source;				//	0：無定義	 1：Local 	2：Spotify
	std::string album;		//	專輯
	std::string artist; 	//	歌手
	std::string song; 		//	歌名
	std::string id; 		//	歌曲ID
	std::string host; 		//	歌曲來源網址
	std::string file; 		//	歌曲檔案名

} RSP_MUSIC;

template<typename T>
class CResponsePacket
{
	typedef std::map<int, T> MAP_DATA;

public:
	CResponsePacket();
	virtual ~CResponsePacket();
	void format(int nType, T &data, JSONObject &jResp);
};
