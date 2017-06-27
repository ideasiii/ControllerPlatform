/*
 * dictionary.cpp
 *
 *  Created on: 2017年6月27日
 *      Author: Jugo
 */

#include "dictionary.h"

map<string, int> mapSubject = create_map<string, int>\
("我", SUBJECT_I)("你", SUBJECT_YOU)("他", SUBJECT_HE)("我們",
SUBJECT_I)("你們", SUBJECT_YOU)("他們", SUBJECT_HE);

map<string, int> mapVerb = create_map<string, int>\
("想要聽", VERB_LISTEN)("超想聽", VERB_LISTEN)("想聽", VERB_LISTEN)("要聽",
VERB_LISTEN)("聽", VERB_LISTEN)("聽聽", VERB_LISTEN);

//=========================== Absolutly ==========================================//
map<string, string> mapAbsolutly;

//=========================== Spotify Music ==========================================//
set<string> setArtist;
set<string> setArtistMark = create_set<string>("Remix");
map<string, string> mapArtistMatch; // TW --> EN

//=========================== Mood Music ==========================================//
// ("歡喜" || "忿怒" || "哀傷" || "驚恐" || "愛情")
map<string, string> mapMood = create_map<string, string>\
("歡喜", "happy.mp3")("忿怒", "angry.mp3")("憤怒", "angry.mp3")(
		"氣憤", "angry.mp3")("不爽", "angry.mp3")("哀傷", "sad.mp3")("悲傷", "sad.mp3")("傷感", "sad.mp3")("驚恐", "shock.mp3")(
		"愛情", "love.mp3");

//=========================== Story ==========================================//
map<string, string> mapStory;
map<string, JSONArray> mapStoryMood;

//=========================== Education ==========================================//
map<string, string> mapEducation;
map<string, string> mapEducationPoetry;

//=========================== Service ==========================================//
map<string, int> mapService;
set<string> setLocation;
