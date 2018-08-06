/*
 * CParticiple.h
 *
 *  Created on: 2018年7月26日
 *      Author: Jugo
 * Description: 文章分詞
 */

#pragma once

#include <set>
#include <string>

class CParticiple
{
public:
	explicit CParticiple();
	virtual ~CParticiple();

public:
	void splitter(const char *szPath, const char *szMark);
};
