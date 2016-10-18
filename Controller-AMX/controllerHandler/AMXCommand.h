/*
 * AMXCommand.h
 *
 *  Created on: 2016年10月18日
 *      Author: Jugo
 */

#pragma once

#include<string>

using namespace std;

#define CTL_OK					"CTL_OK"
#define CTL_ERROR				"CTL_ERROR"

/**
 * {
 "function": 1,
 "device": 0,
 "control": 1
 }

 */
template<typename T, typename U, typename V, typename W>
class map_amx
{
private:
	map<T, map<U, map<V, W> > > m_map;
public:
	map_amx(const T& key1, const U& key2, const V& key3, const W& val)
	{
		map<V, W> m1;
		m1[key3] = val;
		map<U, map<V, W> > m2;
		m2[key2] = m1;
		m_map[key1] = m2;
		m1.clear();
		m2.clear();
	}

	map_amx<T, U, V, W>& operator()(const T& key1, const U& key2, const V& key3, const W& val)
	{
		map<V, W> m1;
		m1[key3] = val;
		map<U, map<V, W> > m2;
		m2[key2] = m1;
		m_map[key1] = m2;
		m1.clear();
		m2.clear();
		return *this;
	}

	operator std::map<T, map<U, map<V, W> > >()
	{
		return m_map;
	}

	string find(const T& key1, const U& key2, const V& key3)
	{
		return m_map.find(key1).find(key2).find(key2);
	}
};

static map<int, map<int, map<int, string> > > mapAMXCommand = map_amx<int, int, int, string>(1, 0, 1, "CTL_SYSTEM_ON");
