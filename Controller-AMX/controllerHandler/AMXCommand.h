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

template<typename T, typename U>
class create_amx_map
{
private:
	std::map<T, U> m_map;
public:
	create_amx_map(const T& key, const U& val)
	{
		m_map[key] = val;
	}

	create_amx_map<T, U>& operator()(const T& key, const U& val)
	{
		m_map[key] = val;
		return *this;
	}

	operator std::map<T, U>()
	{
		return m_map;
	}

};
static map<int, string> mapAMXCommand =
		create_amx_map<int, string>(10001, "CTL_SYSTEM_ON")(10000, "CTL_SYSTEM_OFF")(20100, "CTL_MODE_SPEECH")(20200,
				"CTL_MODE_BRIEF")(20300, "CTL_MODE_CINEMA")(30009, "CTL_MATRIX_INPUT1")(30010, "CTL_MATRIX_INPUT2")(
				30011, "CTL_MATRIX_INPUT3")(30012, "CTL_MATRIX_INPUT4")(30013, "CTL_MATRIX_INPUT5")(30014,
				"CTL_MATRIX_INPUT6")(30015, "CTL_MATRIX_INPUT7")(30016, "CTL_MATRIX_INPUT8")(40101, "CTL_MATRIX_INPUT8")(
				40102, "CTL_PROJ_OFF_LEFT")(40107, "CTL_PROJ_HDMI_LEFT")(40108, "CTL_PROJ_VGA_LEFT")(40103,
				"CTL_PROJ_MUTE_LEFT")(40104, "CTL_PROJ_UNMUTE_LEFT")(40105, "CTL_SCREEN_UP_LEFT")(40106,
				"CTL_SCREEN_DOWN_LEFT")(40201, "CTL_PROJ_ON_CENTER")(40202, "CTL_PROJ_OFF_CENTER")(40207,
				"CTL_PROJ_HDMI_CENTER")(40208, "CTL_PROJ_VGA_CENTER")(40203, "CTL_PROJ_MUTE_CENTER")(40204,
				"CTL_PROJ_UNMUTE_CENTER")(40205, "CTL_SCREEN_UP_CENTER")(40206, "CTL_SCREEN_DOWN_CENTER")(40301,
				"CTL_PROJ_ON_RIGHT")(40302, "CTL_PROJ_OFF_RIGHT")(40307, "CTL_PROJ_HDMI_RIGHT")(40308,
				"CTL_PROJ_VGA_RIGHT")(40303, "CTL_PROJ_MUTE_RIGHT")(40304, "CTL_PROJ_UNMUTE_RIGHT")(40305,
				"CTL_SCREEN_UP_RIGHT")(40306, "CTL_SCREEN_DOWN_RIGHT")(50105, "CTL_INPUT1_VOL_UP")(50106,
				"CTL_INPUT1_VOL_DOWN")(50103, "CTL_INPUT1_MUTE")(50104, "CTL_INPUT1_UNMUTE")(50205, "CTL_INPUT2_VOL_UP")(
				50206, "CTL_INPUT2_VOL_DOWN")(50203, "CTL_INPUT2_MUTE")(50204, "CTL_INPUT2_UNMUTE")(50305,
				"CTL_INPUT2_UNMUTE")(50306, "CTL_INPUT3_VOL_DOWN")(50303, "CTL_INPUT3_MUTE")(50304, "CTL_INPUT3_UNMUTE")(
				50405, "CTL_INPUT4_VOL_UP")(50406, "CTL_INPUT4_VOL_DOWN")(50403, "CTL_INPUT4_MUTE")(50404,
				"CTL_INPUT4_UNMUTE")(50505, "CTL_INPUT5_VOL_UP")(50506, "CTL_INPUT5_VOL_DOWN")(50503, "CTL_INPUT5_MUTE")(
				50504, "CTL_INPUT5_UNMUTE")(50605, "CTL_INPUT6_VOL_UP")(50606, "CTL_INPUT6_VOL_DOWN")(50603,
				"CTL_INPUT6_MUTE")(50604, "CTL_INPUT6_UNMUTE")(50705, "CTL_INPUT7_VOL_UP")(50706, "CTL_INPUT7_VOL_DOWN")(
				50703, "CTL_INPUT7_MUTE")(50704, "CTL_INPUT7_UNMUTE")(50805, "CTL_INPUT9_VOL_UP")(50806,
				"CTL_INPUT9_VOL_DOWN")(50803, "CTL_INPUT9_MUTE")(50804, "CTL_INPUT9_UNMUTE")(50905,
				"CTL_INPUT10_VOL_UP")(50906, "CTL_INPUT10_VOL_DOWN")(50903, "CTL_INPUT10_MUTE")(50904,
				"CTL_INPUT10_UNMUTE")(51005, "CTL_OUTPUT1_VOL_UP")(51006, "CTL_OUTPUT1_VOL_DOWN")(51003,
				"CTL_OUTPUT1_MUTE")(51004, "CTL_OUTPUT1_UNMUTE")(51105, "CTL_OUTPUT2_VOL_UP")(51106,
				"CTL_OUTPUT2_VOL_DOWN")(51103, "CTL_OUTPUT2_MUTE")(51104, "CTL_OUTPUT2_UNMUTE");

//================================================================================================================================
/** deprecated **/
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

//static map<int, map<int, map<int, string> > > mapAMXCommand = map_amx<int, int, int, string>(1, 0, 1, "CTL_SYSTEM_ON");
