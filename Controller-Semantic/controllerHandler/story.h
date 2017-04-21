/*
 * story.h
 *
 *  Created on: 2017年4月21日
 *      Author: root
 */

#pragma once

#include "create_map.h"
#include <string>
#include <map>
using namespace std;

static map<string, string> mapStory = create_map<string, string>\
("三隻小豬", "三隻小豬.mp3")("小美人魚", "小美人魚.mp3")("美人魚",
		"小美人魚.mp3")("睡美人", "睡美人.mp3")("醜小鴨", "醜小鴨.mp3");
