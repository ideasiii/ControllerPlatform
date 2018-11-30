/*
 * CConvert.h
 *
 *  Created on: 2018年11月26日
 *      Author: jugo
 */

#pragma once

class CConvert
{
public:
	explicit CConvert();
	virtual ~CConvert();
	int UTF8toBig5(char *szFrom, char **szTo);
	int Big5toUTF8(char *szFrom, char **szTo);
	int Big5toUTF8(char *szFrom, size_t nFrom, char **szTo);
};
