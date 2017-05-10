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
	VALUE top();
	VALUE low();
	int size();

private:
	std::map<KEY, VALUE> mapData;

};
