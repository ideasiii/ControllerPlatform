/*
 * CTextProcess.h
 *
 *  Created on: 2018年9月28日
 *      Author: Jugo
 */

#pragma once

class CTextProcess
{
public:
	explicit CTextProcess();
	virtual ~CTextProcess();
	void processTheText(const char *szText);
};
