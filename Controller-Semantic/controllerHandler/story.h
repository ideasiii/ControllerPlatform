/*
 * story.h
 *
 *  Created on: 2017年4月21日
 *      Author: root
 */

#pragma once

#include <string>
#include <map>
using namespace std;

template<typename T, typename U>
class create_map
{
private:
	std::map<T, U> m_map;
public:
	create_map(const T& key, const U& val)
	{
		m_map[key] = val;
	}

	create_map<T, U>& operator()(const T& key, const U& val)
	{
		m_map[key] = val;
		return *this;
	}

	operator std::map<T, U>()
	{
		return m_map;
	}

};

static map<string, string> mapStory = create_map<string, string>\
("三隻小豬", "三隻小豬.mp3")("小美人魚", "小美人魚.mp3")("美人魚",
		"小美人魚.mp3")("睡美人", "睡美人.mp3")("醜小鴨", "醜小鴨.mp3");
