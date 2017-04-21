/*
 * mood.h
 *
 *  Created on: 2017年4月21日
 *      Author: Jugo
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

//======================== Local Mood Music ==============================//
// ("歡喜" || "忿怒" || "哀傷" || "驚恐" || "愛情")
static map<string, string> mapMood =
		create_map<string, string>\
("歡喜", "happy.mp3")("忿怒", "angry.mp3")("哀傷", "sad.mp3")("驚恐", "shock.mp3")("愛情",
				"love.mp3");
