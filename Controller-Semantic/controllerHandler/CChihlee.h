/*
 * CChihlee.h
 *
 *  Created on: 2019年3月15日
 *      Author: jugo
 */

#pragma once

#include <map>
#include <list>

class JSONObject;
class CMysqlHandler;


class CChihlee
{
public:
	explicit CChihlee();
	virtual ~CChihlee();
	void runAnalysis(const char *szInput, JSONObject &jsonResp);
	void init();
	void setMySQLIP(const char * szIP);
	void setWordPath(const char * szPath);

private:
	void playSound(const char *szWav);
	void intent(int nIntent, int nType, const char* szWord,JSONObject &jsonResp, CMysqlHandler*& mysql);
	std::string course(int nType, const char* szWord, CMysqlHandler*& mysql);
	std::string office(int nType, const char* szWord, CMysqlHandler*& mysql);
	void displayWord(const char * szWord);
	void fuzzyWord(std::string &strWord);


private:
	std::string m_strWordPath;
	std::string m_strMySQLIP;
	//CMysqlHandler *mysql;
	std::list<std::map<std::string, std::string> > listKeyWord;
	std::map<std::string,std::string> mapFuzzyWord; // fuzzy ---> correct

};
