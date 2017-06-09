/*
 * dictionaty.h
 *
 *  Created on: 2017年6月3日
 *      Author: Jugo
 */

#pragma once

#include "container.h"

//=========================== Semantic ==========================================//
/**
 *  主詞
 */
#define SUBJECT_I			1
#define SUBJECT_YOU			2
#define SUBJECT_HE			3

/**
 *  動詞
 */
#define VERB_LISTEN			1

static map<string, int> mapSubject = create_map<string, int>\
("我", SUBJECT_I)("你", SUBJECT_YOU)("他", SUBJECT_HE)("我們",
SUBJECT_I)("你們", SUBJECT_YOU)("他們", SUBJECT_HE);

static map<string, int> mapVerb = create_map<string, int>\
("想要聽", VERB_LISTEN)("超想聽", VERB_LISTEN)("想聽", VERB_LISTEN)(
		"要聽", VERB_LISTEN)("聽", VERB_LISTEN)("聽聽", VERB_LISTEN);

//=========================== Absolutly ==========================================//
static map<string, string> mapAbsolutly;

//=========================== Spotify Music ==========================================//
static set<string> setArtist;
static set<string> setArtistMark = create_set<string>("(Remastered)")("(Live)")("(Deluxe Remaster)")("(Deluxe)");
static map<string, string> mapArtistMatch; // TW --> EN

//=========================== Mood Music ==========================================//
// ("歡喜" || "忿怒" || "哀傷" || "驚恐" || "愛情")
static map<string, string> mapMood = create_map<string, string>\
("歡喜", "happy.mp3")("忿怒", "angry.mp3")("憤怒",
		"angry.mp3")("氣憤", "angry.mp3")("不爽", "angry.mp3")("哀傷", "sad.mp3")("悲傷", "sad.mp3")("傷感", "sad.mp3")("驚恐",
		"shock.mp3")("愛情", "love.mp3");

//=========================== Story ==========================================//
static map<string, string> mapStory;

//=========================== Education ==========================================//
static map<string, string> mapEducation;
static map<string, string> mapEducationPoetry;

//==================================== match_service ===========================================//

#define SERVICE_CLOCK				1
#define SERVICE_WEATHER				2
static map<string, int> mapService;

