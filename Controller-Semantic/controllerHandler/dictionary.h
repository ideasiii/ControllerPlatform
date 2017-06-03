/*
 * dictionaty.h
 *
 *  Created on: 2017年6月3日
 *      Author: Jugo
 */

#pragma once

#include "container.h"

//=========================== Semantic ==========================================//
#define SUBJECT_I			1
#define SUBJECT_YOU			2
#define SUBJECT_HE			3

#define VERB_LISTEN			1

static map<string, int> mapSubject = create_map<string, int>\
("我", SUBJECT_I)("你", SUBJECT_YOU)("他", SUBJECT_HE)("我們",
SUBJECT_I)("你們", SUBJECT_YOU)("他們", SUBJECT_HE);

static map<string, int> mapVerb = create_map<string, int>\
("想要聽", VERB_LISTEN)("超想聽", VERB_LISTEN)("想聽", VERB_LISTEN)(
		"要聽", VERB_LISTEN)("聽", VERB_LISTEN)("聽聽", VERB_LISTEN);

//=========================== Spotify Music ==========================================//
static set<string> setArtistEnglishMale;
static set<string> setArtistEnglishFemale;
static set<string> setArtistTaiwan;
static set<string> setArtistHardRock;
static set<string> setArtistHongKong;
static set<string> setArtistGuitarist;

static map<string, set<string> > mapArtistDic = create_map<string, set<string> >("dictionary/artist_en_male.txt",
		setArtistEnglishMale)("dictionary/artist_en_female.txt", setArtistEnglishFemale)("dictionary/artist_tw.txt",
		setArtistTaiwan)("dictionary/artist_en_hard_rock.txt", setArtistHardRock)("dictionary/artist_hk.txt",
		setArtistHongKong)("dictionary/artist_en_guitarist.txt", setArtistGuitarist);

static set<string> setMark = create_set<string>("(Remastered)")("(Live)")("(Deluxe Remaster)")("(Deluxe)");

//=========================== Mood Music ==========================================//
// ("歡喜" || "忿怒" || "哀傷" || "驚恐" || "愛情")
static map<string, string> mapMood = create_map<string, string>\
("歡喜", "happy.mp3")("忿怒", "angry.mp3")("憤怒",
		"angry.mp3")("氣憤", "angry.mp3")("不爽", "angry.mp3")("哀傷", "sad.mp3")("悲傷", "sad.mp3")("傷感", "sad.mp3")("驚恐",
		"shock.mp3")("愛情", "love.mp3");

//=========================== Story ==========================================//
static map<string, string> mapStory;
