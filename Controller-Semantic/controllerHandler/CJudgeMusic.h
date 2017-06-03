/*
 * CJudgeMusic.h
 *
 *  Created on: 2017年5月11日
 *      Author: Jugo
 */

#pragma once


#include "CSemantic.h"

class JSONObject;

class CJudgeMusic: public CSemantic
{
public:
	explicit CJudgeMusic();
	virtual ~CJudgeMusic();
	int word(const char *szInput, JSONObject* jsonResp);
	int evaluate(const char *szWord);

private:
	typedef std::string (CJudgeMusic::*pFnGetArtist)(const char *);
	std::map<int, pFnGetArtist> mappFnGetArtist;
	std::set<std::set<std::string> > setArtist;
	std::string getArtist(const char *szWord);
	void loadArtistDictionary();
};
