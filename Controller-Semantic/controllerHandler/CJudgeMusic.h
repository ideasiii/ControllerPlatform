/*
 * CJudgeMusic.h
 *
 *  Created on: 2017年5月11日
 *      Author: Jugo
 */

#pragma once

#include "CSemantic.h"

class JSONObject;
class CSpotify;

class CJudgeMusic: public CSemantic
{
public:
	explicit CJudgeMusic();
	virtual ~CJudgeMusic();

protected:
	std::string toString();
	int word(const char *szInput, JSONObject* jsonResp, std::map<std::string, std::string> &mapMatch);
	int evaluate(const char *szWord, std::map<std::string, std::string> &mapMatch);

private:
	std::string getArtist(const char *szWord);
	void loadArtistDictionary();
	CSpotify *spotify;
};
