/*
 * dictionary.h
 *
 *  Created on: 2017年4月19日
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

static map<string, int> mapSubject = create_map<string, int>\
("我", 1)("你", 2)("他", 3)("我們", 1)("你們", 2)("他們", 3);

