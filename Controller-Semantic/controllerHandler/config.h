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
	CONTENT_STORY = 0,
	CONTENT_MUSIC_SPOTIFY,
	CONTENT_MUSIC_MOOD,
	CONTENT_ABSOLUTELY,
	CONTENT_EDUCATION,
	CONTENT_SERVICE,
	CONTENT_TRANSLATE
} CONTENT_TYPE;

//==================================== 封包 Request 內容定義 =========================================//
enum
{
	TYPE_REQ_NODEFINE = 0,
	TYPE_REQ_CONTROL,
	TYPE_REQ_TALK,
	TYPE_REQ_RECORD,
	TYPE_REQ_STORY,
	TYPE_REQ_GAME,
	TYPE_REQ_PEN,
	TYPE_REQ_MAX
};

//==================================== Content Service =========================================//
enum
{
	SERVICE_NO = 0, SERVICE_SPOTIFY = 1, SERVICE_WEATHER = 2
};

//==================================== 封包 Response 內容定義 =========================================//
#define TYPE_RESP_UNKNOW			0x00000000
#define TYPE_RESP_MUSIC_SPOTIFY		0x00000001
#define TYPE_RESP_STORY				0x00000002
#define TYPE_RESP_TTS				0x00000003
#define TYPE_RESP_MUSIC_LOCAL		0x00000004

#define STORY_PATH					"/data/opt/tomcat/webapps/story/"
#define PATH_STORY_MOOD				"/data/opt/tomcat/webapps/edubot/mood/"
#define STORY_HOST					"https://smabuild.sytes.net/story/"
#define MUSIC_LOCAL_HOST			"https://smabuild.sytes.net/music/local/"
#define HOST_MOOD					"https://smabuild.sytes.net/edubot/mood/"

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
#define SERVICE_TRANSLATE			3

//=========================== Semantic ==========================================//
/**
 *  主詞
 */
#define SUBJECT_I					1
#define SUBJECT_YOU					2
#define SUBJECT_HE					3

/**
 *  動詞
 */
#define VERB_LISTEN					1
#define VERB_SAY					2
#define VERB_READ					3
#define VERB_WRITE					4
#define VERB_REQUEST				5

