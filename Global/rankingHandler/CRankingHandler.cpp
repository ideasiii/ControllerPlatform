/*
 * CRankingHandler.cpp
 *
 *  Created on: 2017年5月4日
 *      Author: Jugo
 */

#include <map>
#include <vector>
#include "CRankingHandler.h"

using namespace std;

template<typename KEY, typename VALUE>
CRankingHandler<KEY, VALUE>::CRankingHandler()
{

}

template<typename KEY, typename VALUE>
CRankingHandler<KEY, VALUE>::~CRankingHandler()
{
	mapData.clear();
}

template<typename KEY, typename VALUE>
multimap<VALUE, KEY> CRankingHandler<KEY, VALUE>::flip_map(map<KEY, VALUE> & src)
{
	multimap<VALUE, KEY> dst;
	typename map<KEY, VALUE>::iterator iter;

	for(iter = src.begin(); iter != src.end(); ++iter)
		dst.insert(pair<VALUE, KEY>(iter->second, iter->first));

	return dst;
}

template<typename KEY, typename VALUE>
void CRankingHandler<KEY, VALUE>::add(KEY key, VALUE value)
{
	mapData[key] = value;
}

template<typename KEY, typename VALUE>
VALUE CRankingHandler<KEY, VALUE>::topValue()
{
	if(!size())
	{
		return 0;
	}
	multimap<VALUE, KEY> dst;

	dst = flip_map(mapData);

	typename multimap<VALUE, KEY>::const_reverse_iterator it = dst.rbegin();
	return it->first;
}

template<typename KEY, typename VALUE>
VALUE CRankingHandler<KEY, VALUE>::lowValue()
{
	if(!size())
	{
		return 0;
	}
	multimap<VALUE, KEY> dst;

	dst = flip_map(mapData);

	typename multimap<VALUE, KEY>::const_iterator it = dst.begin();
	return it->first;
}

template<typename KEY, typename VALUE>
KEY CRankingHandler<KEY, VALUE>::topValueKey()
{
	if(!size())
	{
		return 0;
	}
	multimap<VALUE, KEY> dst;

	dst = flip_map(mapData);

	typename multimap<VALUE, KEY>::const_reverse_iterator it = dst.rbegin();

	return it->second;
}

template<typename KEY, typename VALUE>
void CRankingHandler<KEY, VALUE>::topValueKeys(vector<KEY> &keys)
{
	VALUE topVal;

	topVal = topValue();
	for(typename map<KEY, VALUE>::iterator it_map = mapData.begin(); mapData.end() != it_map; ++it_map)
	{
		if(it_map->second == topVal)
		{
			keys.push_back(it_map->first);
		}
	}

}

template<typename KEY, typename VALUE>
KEY CRankingHandler<KEY, VALUE>::lowValueKey()
{
	if(!size())
	{
		return 0;
	}
	multimap<VALUE, KEY> dst;

	dst = flip_map(mapData);

	typename multimap<VALUE, KEY>::const_iterator it = dst.begin();
	return it->second;
}

template<typename KEY, typename VALUE>
int CRankingHandler<KEY, VALUE>::size()
{
	return mapData.size();
}

template<typename KEY, typename VALUE>
VALUE CRankingHandler<KEY, VALUE>::getValue(KEY key, VALUE defVal)
{
	if(mapData.end() != mapData.find(key))
	{
		return mapData[key];
	}
	return defVal;
}

