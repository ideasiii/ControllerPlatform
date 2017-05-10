/*
 * CRankingHandler.cpp
 *
 *  Created on: 2017年5月4日
 *      Author: Jugo
 */

#include <map>
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
VALUE CRankingHandler<KEY, VALUE>::top()
{
	if(!size())
	{
		VALUE nopos;
		return nopos;
	}
	multimap<VALUE, KEY> dst;

	dst = flip_map(mapData);

	typename multimap<VALUE, KEY>::const_reverse_iterator it = dst.rbegin();
	return it->first;
}

template<typename KEY, typename VALUE>
VALUE CRankingHandler<KEY, VALUE>::low()
{
	if(!size())
	{
		VALUE nopos;
		return nopos;
	}
	multimap<VALUE, KEY> dst;

	dst = flip_map(mapData);

	typename multimap<VALUE, KEY>::const_iterator it = dst.begin();
	return it->first;
}

template<typename KEY, typename VALUE>
int CRankingHandler<KEY, VALUE>::size()
{
	return mapData.size();
}

