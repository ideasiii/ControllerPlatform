/*
 * config.h
 *
 *  Created on: 2017年4月19日
 *      Author: Jugo
 */

#pragma once

//==================================== 內容定義 =========================================//
#define CONTENT_STORY				0x00000001
#define CONTENT_MUSIC				0x00000002

//==================================== 封包 Request 內容定義 =========================================//
#define TYPE_REQ_NODEFINE			0x00000000
#define TYPE_REQ_CONTROL			0x00000001
#define TYPE_REQ_TALK				0x00000002
#define TYPE_REQ_RECORD				0x00000003

//==================================== 封包 Response 內容定義 =========================================//
#define TYPE_RESP_UNKNOW			0x00000000
#define TYPE_RESP_MUSIC				0x00000001
#define TYPE_RESP_STORY				0x00000002
#define TYPE_RESP_TTS				0x00000003

#define STORY_HOST					"https://iiideasmartbuilding.sytes.net/story/"
#define MUSIC_LOCAL_HOST			"https://iiideasmartbuilding.sytes.net/music/local/"

//==================================== 字詞屬性定義 ========================================//
#define UNKNOW						0x00000000
#define SUBJECT						0x00000001		// 主詞
#define VERB						0x00000002		// 動詞
#define NOUN						0x00000003		// 名詞
#define ADJECTIVE					0x00000004		// 形容詞

