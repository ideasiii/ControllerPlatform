/*
 * CCmpWord.h
 *
 *  Created on: 2017年4月10日
 *      Author: root
 */

#pragma once

#include "CCmpServer.h"

class CCmpWord: public CCmpServer
{
	typedef struct _WORD_REQUEST
	{
		int nId;
		int nType;
		int nTotal;
		int nNumber;
		std::string strWord;
	} WORD_REQUEST;
public:
	explicit CCmpWord();
	virtual ~CCmpWord();

protected:
	int onSemanticWord(int nSocket, int nCommand, int nSequence, const void *szData);
};
