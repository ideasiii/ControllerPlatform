/*
 * CAnalysisHandler.cpp
 *
 *  Created on: 2017年7月18日
 *      Author: Jugo
 */

#include <string>
#include "common.h"
#include "CAnalysisHandler.h"
#include "LogHandler.h"
#include "CConfig.h"
#include "CFileHandler.h"
#include "utility.h"
#include "config.h"
#include "dataStruct.h"
#include "CResponsePacket.h"
#include "CContentHandler.h"
#include "CDisplayHandler.h"

using namespace std;

CAnalysisHandler::CAnalysisHandler(const char *szConf, CObject *object) :
		contentHandler(0)
{
	m_pParent = object;

	if(szConf && loadConf(szConf))
	{
		contentHandler = new CContentHandler;
		loadData();
	}

	mapFunc[SERVICE_SPOTIFY] = &CAnalysisHandler::serviceSpotify;
	mapFunc[SERVICE_WEATHER] = &CAnalysisHandler::serviceWeather;
	mapFunc[SERVICE_NEWS] = &CAnalysisHandler::serviceNews;
}

CAnalysisHandler::~CAnalysisHandler()
{
	mapData.clear();
	if(contentHandler)
		delete contentHandler;
}

bool CAnalysisHandler::loadConf(const char *szConf)
{
	bool bResult;
	string strValue;
	CConfig *config;

	bResult = false;

	if(szConf)
	{
		_log("[CAnalysisHandler] CAnalysisHandler Load conf file: %s", szConf);

		config = new CConfig();
		if(config->loadConfig(szConf))
		{
			conf.strName = config->getValue("CONF", "name");
			conf.strWordUnknow = config->getValue("WORD", "unknow");
			conf.strWordError = config->getValue("WORD", "error");
			if(!(strValue = config->getValue("CONF", "type")).empty())
				convertFromString(conf.nType, strValue);

			if(!(strValue = config->getValue("CONTENT", "service")).empty())
				convertFromString(conf.nService, strValue);

			_log("[CAnalysisHandler] loadConf type: %d  service: %d", conf.nType, conf.nService);

			switch(conf.nType)
			{
			case CONF_TYPE_LOCAL_FILE:
				conf.strPath = config->getValue("FILE", "path");
				conf.strFileType = config->getValue("FILE", "type");
				conf.strHost = config->getValue("FILE", "host");
				conf.strDisplayPath = config->getValue("DISPLAY", "path");
				bResult = true;
				break;
			case CONF_TYPE_DICTIONARY:
				conf.strDictionary = config->getValue("DICTIONARY", "file");
				bResult = true;
				break;
			case CONF_TYPE_UNDEFINE:
				bResult = true;
				break;
			default:
				bResult = false;
				_log("[CAnalysisHandler] CAnalysisHandler Unknow config type: %d", conf.nType);
				break;
			}
			_log("[CAnalysisHandler] CAnalysisHandler get conf name: %s type: %d", conf.strName.c_str(), conf.nType);

			loadKeyWord(config->getValue("KEY", "word").c_str());
			loadVerb(config->getValue("VOCABULARY", "verb").c_str());
			loadMatch(config->getValue("MATCH", "file").c_str());
		}
		delete config;
	}

	return bResult;
}

void CAnalysisHandler::loadKeyWord(const char *szWord)
{
	vector<string> vData;

	if(!szWord)
		return;

	setKeyWord.clear();
	spliteData(const_cast<char*>(szWord), ",", setKeyWord);

	for(set<string>::const_iterator it = setKeyWord.begin(); setKeyWord.end() != it; ++it)
	{
		_log("[CAnalysisHandler] loadKeyWord key word: %s", it->c_str());
	}
}

void CAnalysisHandler::loadVerb(const char *szWord)
{
	int nValue;

	if(!szWord)
		return;

	convertFromString(nValue, szWord);
	vocabulary.nVerb = nValue;
}

string CAnalysisHandler::getName()
{
	return conf.strName;
}

void CAnalysisHandler::loadData()
{
	switch(conf.nType)
	{
	case CONF_TYPE_LOCAL_FILE:
		loadLocalFile();
		break;
	case CONF_TYPE_DICTIONARY:
		loadDictionary();
		break;
	}
}

void CAnalysisHandler::loadLocalFile()
{
	int nIndex;
	string strFileName;
	string strDisplayFile;
	CFileHandler fh;
	set<string> setData;
	set<string>::const_iterator iter_set;
	map<string, string>::const_iterator iter_map;

	fh.readPath(conf.strPath.c_str(), setData);
	setDictionary.clear();
	for(iter_set = setData.begin(); setData.end() != iter_set; ++iter_set)
	{
		nIndex = iter_set->rfind(".");
		if((int) string::npos != nIndex)
		{
			if(!iter_set->substr(nIndex + 1).compare(conf.strFileType))
			{
				strFileName = trim(iter_set->substr(0, nIndex));
				setDictionary.insert(trim(strFileName));
				mapData[strFileName].udata.localData.strName = strFileName;
				mapData[strFileName].udata.localData.strPath = conf.strPath + (*iter_set);
				mapData[strFileName].udata.localData.strType = conf.strFileType;
				mapData[strFileName].udata.localData.strHost = conf.strHost + (*iter_set);
				mapData[strFileName].udata.localData.strDisplayFile = conf.strDisplayPath + strFileName + ".display";
			}
		}
	}

	_log("[CAnalysisHandler] loadLocalFile: %s file count: %d", conf.strName.c_str(), setDictionary.size());
//	for(map<string, RESOURCE>::const_iterator it = mapData.begin(); mapData.end() != it; ++it)
//		_log("%s - %s", it->second.udata.localData.strHost.c_str(), it->second.udata.localData.strDisplayFile.c_str());

}

void CAnalysisHandler::loadMatch(const char *szPath)
{
	CFileHandler fh;
	set<string> setData;
	set<string>::const_iterator iter_set;

	if(!szPath)
		return;

	mapMatchWord.clear();
	fh.readAllLine(szPath, setData);
	for(iter_set = setData.begin(); setData.end() != iter_set; ++iter_set)
	{
		if(!iter_set->empty())
		{
			mapMatchWord[iter_set->substr(0, iter_set->find(","))] = iter_set->substr(iter_set->find(",") + 1);
//			_log("[CAnalysisHandler] loadMatch %s <---> %s", iter_set->substr(0, iter_set->find(",")).c_str(),
//					mapMatchWord[iter_set->substr(0, iter_set->find(","))].c_str());
		}
	}

	_log("[CAnalysisHandler] loadMatch: %s match count: %d", conf.strName.c_str(), mapMatchWord.size());
}

void CAnalysisHandler::loadDictionary()
{
	int nIndex;
	string strFileName;
	string strDisplayFile;
	CFileHandler fh;
	set<string>::const_iterator iter_set;
	map<string, string>::const_iterator iter_map;

	setDictionary.clear();
	fh.readAllLine(conf.strDictionary.c_str(), setDictionary);

	_log("[CAnalysisHandler] loadDictionary: %s dictionary count: %d", conf.strName.c_str(), setDictionary.size());
}

int CAnalysisHandler::evaluate(const char *szWord, std::map<std::string, std::string> &mapMatch)
{
	int nScore;
	string strWord;
	string strValue;
	string strTmp;
	set<string>::const_iterator it_set;
	map<string, string>::const_iterator iter_map;

	strWord = szWord;
	if(strWord.empty())
		return 0;

	nScore = 0;
	transform(strWord.begin(), strWord.end(), strWord.begin(), ::tolower);

	//======== 評估關鍵字 ==========//
	for(it_set = setKeyWord.begin(); setKeyWord.end() != it_set; ++it_set)
	{
		strTmp = trim(*it_set);
		transform(strTmp.begin(), strTmp.end(), strTmp.begin(), ::tolower);
		if(string::npos != strWord.find(strTmp))
		{
			++nScore;
			_log("[CAnalysisHandler] evaluate find key word: %s", it_set->c_str());
			if(CONF_TYPE_UNDEFINE == conf.nType) //	int word(const char *szInput, JSONObject& jsonResp);
			{
				mapMatch["dictionary"] = *it_set;
			}
		}
	}

	//======== 評估字典檔 ==========//
	strValue.clear();
	for(it_set = setDictionary.begin(); setDictionary.end() != it_set; ++it_set)
	{
		strTmp = trim(*it_set);
		transform(strTmp.begin(), strTmp.end(), strTmp.begin(), ::tolower);
		if(string::npos != strWord.find(strTmp))
		{
			if(strValue.empty() || strValue.length() < it_set->length())
			{
				strValue = trim(*it_set);
			}
		}
	}

	if(!strValue.empty())
	{
		mapMatch["dictionary"] = strValue;
		_log("[CAnalysisHandler] evaluate find dictionary word: %s", mapMatch["dictionary"].c_str());
		++nScore;
	}

	//======== 評估相似詞 ===========//
	strValue.clear();
	for(iter_map = mapMatchWord.begin(); mapMatchWord.end() != iter_map; ++iter_map)
	{
		strTmp = trim(iter_map->first);
		transform(strTmp.begin(), strTmp.end(), strTmp.begin(), ::tolower);
		if(string::npos != strWord.find(strTmp))
		{
			mapMatch["fuzzy"] = trim(iter_map->second);
			_log("[CAnalysisHandler] evaluate fuzzy word: %s --> %s", strTmp.c_str(), mapMatch["fuzzy"].c_str());
		}
	}

	if(!strValue.empty())
	{
		mapMatch["dictionary"] = strValue;
		_log("[CAnalysisHandler] evaluate find match word: %s", mapMatch["dictionary"].c_str());
		++nScore;
	}

	//======== 評估詞彙 ===========//
	WORD_ATTR wordAttr;
	if(0 <= getVerb(strWord.c_str(), wordAttr))
	{
		if(vocabulary.nVerb == wordAttr.nSubAttr)
		{
			++nScore;
			_log("[CAnalysisHandler] evaluate VERB: %s", wordAttr.strWord.c_str());
		}
	}

	return nScore;
}

int CAnalysisHandler::activity(const char *szInput, JSONObject& jsonResp, map<string, string> &mapMatch)
{
	string strDisplay;
	string strFileName;
	CResponsePacket respPacket;

	_log("[CAnalysisHandler] activity mapMatch: %s", mapMatch["dictionary"].c_str());

	switch(conf.nType)
	{
	case CONF_TYPE_LOCAL_FILE:
		if(!mapMatch["dictionary"].empty() && (mapData.end() != mapData.find(mapMatch["dictionary"])))
		{
			strFileName = mapData[mapMatch["dictionary"]].udata.localData.strName + "."
					+ mapData[mapMatch["dictionary"]].udata.localData.strType;
			_log("[CAnalysisHandler] activity load local file: %s",
					mapData[mapMatch["dictionary"]].udata.localData.strPath.c_str());
			respPacket.setActivity("type", 1).setActivity("host", conf.strHost).setActivity("file", strFileName);
			//strDisplay = getDisplay(mapData[mapMatch["dictionary"]].udata.localData.strDisplayFile.c_str());
//			if(!strDisplay.empty())
//			{
//				respPacket.setDisplay(strDisplay.c_str());
//			}
			respPacket.format(jsonResp);
		}
		else
		{
			if(!conf.strName.compare("story"))
			{
				respPacket.setActivity<int>("type", RESP_TTS).setActivity<const char*>("lang", "zh").setActivity<
						const char*>("tts", "無此相關情境與主題的故事喔").format(jsonResp);
			}
		}
		break;
	case CONF_TYPE_DICTIONARY:
	case CONF_TYPE_UNDEFINE:
		if(conf.nService)
			service(szInput, jsonResp, mapMatch);
		break;
	}
	return 0;
}

int CAnalysisHandler::service(const char *szInput, JSONObject& jsonResp, std::map<std::string, std::string> &mapMatch)
{
	if(mapFunc.end() == mapFunc.find(conf.nService))
	{
		_log("[CAnalysisHandler] service Function pointer not found :%d", conf.nService);
		return FALSE;
	}

	(this->*this->mapFunc[conf.nService])(szInput, mapMatch["dictionary"].c_str(), jsonResp);

	return TRUE;
}

void CAnalysisHandler::serviceSpotify(const char *szWord, const char *szArtist, JSONObject& jsonResp)
{
	string strDisplay;
	TRACK track;
	CResponsePacket respPacket;
	CDisplayHandler display;

	if(contentHandler->spotifyTrack(szWord, szArtist, track))
	{
		strDisplay =
				"{\"enable\":1,\"show\":[{\"time\":0,\"host\":\"" + track.strCover
						+ "\",\"file\":\"\",\"color\":\"#FFFFFFFF\",\"description\":\"Cover\",\"animation\":{\"type\":0,\"duration\":0,\"repeat\":0,\"interpolate\":0},\"text\":{\"type\":0}}]}";
		respPacket.setActivity<int>("type", RESP_SPOTIFY).setActivity<const char*>("id", track.id.c_str()).setDisplay(
				strDisplay.c_str()).format(jsonResp);
	}
	else
	{
		respPacket.setActivity<int>("type", RESP_TTS).setActivity<const char*>("lang", "zh").setActivity<const char*>(
				"tts", "無此歌手的樂曲或此歌手未授權播放").setDisplay(display.getSadDisplay().c_str()).format(jsonResp);
	}
}

void CAnalysisHandler::serviceWeather(const char *szWord, const char *szLocal, JSONObject& jsonResp)
{
	string strLocal;
	string strTTS;
	WEATHER weather;
	CResponsePacket respPacket;
	CDisplayHandler display;

	strLocal = szLocal;
	if(strLocal.empty())
		strLocal = "台灣";

	contentHandler->getWeather(strLocal.c_str(), weather);

	if(!weather.strLocation.empty())
		strTTS = format("現在%s天氣，%s;氣溫%.02f度;溼度%.02f度", weather.strLocation.c_str(), weather.strWeather.c_str(),
				weather.fTemperature, weather.fHumidity);
	else
		strTTS = format("目前無法取得 %s 的天氣資訊", strLocal.c_str());

	respPacket.setActivity<int>("type", RESP_TTS).setActivity<const char*>("lang", "zh").setActivity<const char*>("tts",
			strTTS.c_str()).setDisplay(display.getSadDisplay().c_str()).format(jsonResp);
}

void CAnalysisHandler::serviceNews(const char *szWord, const char *szLocal, JSONObject& jsonResp)
{
	static NEWS_DATE newsDate;
	contentHandler->getNews(newsDate);
}

string CAnalysisHandler::getDisplay(const char *szFile)
{
	string strContent;
	CFileHandler fh;
	CDisplayHandler display;

	if(szFile)
	{
		if(!fh.readContent(szFile, strContent))
		{
			strContent = display.getDefaultDisplay();
		}
	}

	return strContent;
}

string CAnalysisHandler::getWord(enuWORD ew)
{
	switch(ew)
	{
	case WORD_UNKNOW:
		return conf.strWordUnknow;
	case WORD_ERROR:
		return conf.strWordError;
	}
	return GLOBAL_WORD_UNKNOW;
}

