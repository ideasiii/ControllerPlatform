/*
 * mood.h
 *
 *  Created on: 2017年4月21日
 *      Author: Jugo
 */

#pragma once

#include "create_map.h"
#include <string>
#include <map>
using namespace std;

//======================== Local Mood Music ==============================//
// ("歡喜" || "忿怒" || "哀傷" || "驚恐" || "愛情")
static map<string, string> mapMood = create_map<string, string>\
("歡喜", "happy.mp3")("忿怒", "angry.mp3")("憤怒",
		"angry.mp3")("氣憤", "angry.mp3")("不爽", "angry.mp3")("哀傷", "sad.mp3")("悲傷", "sad.mp3")("傷感", "sad.mp3")("驚恐",
		"shock.mp3")("愛情", "love.mp3");
