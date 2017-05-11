/*
 * CRankingHandler.h
 *
 *  Created on: 2017年5月4日
 *      Author: Jugo
 *
 *  說明:做MAP排序用
 */

#pragma once

template<typename KEY, typename VALUE>
class CRankingHandler
{
public:
	CRankingHandler();
	virtual ~CRankingHandler();
	std::multimap<VALUE, KEY> flip_map(std::map<KEY, VALUE> & src);
	void add(KEY, VALUE);
	VALUE topValue();
	VALUE lowValue();
	KEY topValueKey();
	KEY lowValueKey();
	int size();

private:
	std::map<KEY, VALUE> mapData;

};
