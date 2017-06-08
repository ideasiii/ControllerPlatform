/*
 * config.h
 *
 *  Created on: 2017年4月19日
 *      Author: Jugo
 */

#pragma once

//==================================== 內容定義 =========================================//
typedef enum _CONTENT_TYPE
{
	CONTENT_STORY = 0, CONTENT_MUSIC_SPOTIFY, CONTENT_MUSIC_MOOD, CONTENT_ABSOLUTELY, CONTENT_EDUCATION
} CONTENT_TYPE;

//==================================== 封包 Request 內容定義 =========================================//
#define TYPE_REQ_NODEFINE			0x00000000
#define TYPE_REQ_CONTROL			0x00000001
#define TYPE_REQ_TALK				0x00000002
#define TYPE_REQ_RECORD				0x00000003
#define TYPE_REQ_MAX				0x00000004

//==================================== 封包 Response 內容定義 =========================================//
#define TYPE_RESP_UNKNOW			0x00000000
#define TYPE_RESP_MUSIC				0x00000001
#define TYPE_RESP_STORY				0x00000002
#define TYPE_RESP_TTS				0x00000003

#define STORY_PATH					"/data/opt/tomcat/webapps/story"
#define STORY_HOST					"https://iiideasmartbuilding.sytes.net/story/"
#define MUSIC_LOCAL_HOST			"https://iiideasmartbuilding.sytes.net/music/local/"

//==================================== 字詞屬性定義 ========================================//
#define UNKNOW						0x00000000
#define SUBJECT						0x00000001		// 主詞
#define VERB						0x00000002		// 動詞
#define NOUN						0x00000003		// 名詞
#define ADJECTIVE					0x00000004		// 形容詞

//==================================== Spotify ========================================//
#define SPOTIFY_CLIENT				"MzgyOTVjMmY1ZDNjNDM3MWI4MTFkZGZiMDA3NWRlODE6MWI4OWQxN2QyNjg4NDRkZGIyNWUzMWRiZmE1NzQzZjE="

//==================================== 固定回復 ===========================================//
#define WORD_UNKNOW					"我不了解這句話的意思"

//==================================== match_service ===========================================//

#define SERVICE_CLOCK				1
#define SERVICE_WEATHER				2

