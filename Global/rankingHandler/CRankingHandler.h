/*
 * CRankingHandler.h
 *
 *  Created on: 2017年5月4日
 *      Author: Jugo
 */

#pragma once

#include <map>

template<typename KEY, typename VALUE>
class CRankingHandler
{
	typedef std::map<KEY, VALUE> MAP_DATA;
public:
	CRankingHandler();
	virtual ~CRankingHandler();
};
