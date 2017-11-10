/*
 * CSemanticJudge.cpp
 *
 *  Created on: 2017年4月19日
 *      Author: Jugo
 */

#include <map>
#include <string>
#include <algorithm>
#include "CSemanticJudge.h"
#include "JSONObject.h"
#include "config.h"
#include "common.h"
#include "Handler.h"
#include "CObject.h"
#include "CRankingHandler.cpp"
#include "CSemantic.h"
#include "CResponsePacket.h"
#include "CAnalysisHandler.h"
#include "CConfig.h"
#include "utility.h"
#include "CString.h"
#include "CDisplayHandler.h"
#include "CStory.h"

using namespace std;

CSemanticJudge::CSemanticJudge(CObject *object)
{
	mpController = object;
}

CSemanticJudge::~CSemanticJudge()
{
	for(unsigned int i = 1; i <= mapAnalysis.size(); ++i)
	{
		delete mapAnalysis[i];
	}
	mapAnalysis.clear();
}

void CSemanticJudge::loadAnalysis()
{
	int nValue;
	string strValue;
	CConfig *config;

	config = new CConfig();
	if(config->loadConfig(getConfigFile()))
	{
		strValue = config->getValue("ANALYSIS", "total");
		convertFromString(nValue, strValue);
		_log("[CSemanticJudge] CSemanticJudge get analysis total: %d", nValue);
		for(int i = 1; i <= nValue; ++i)
		{
			strValue = config->getValue("ANALYSIS", ConvertToString(i));
			_log("[CSemanticJudge] CSemanticJudge Get conf: %s conf file: %s", ConvertToString(i).c_str(),
					strValue.c_str());
			if(!strValue.empty())
				mapAnalysis[i] = new CAnalysisHandler(strValue.c_str(), this);
		}
	}
	else
	{
		_log("[CSemanticJudge] CSemanticJudge Load Configure Fail, File: %s", getConfigFile().c_str());
	}

	delete config;

	mapSemanticService[0] = new CStory();

	for(unsigned int i = 0; i < mapSemanticService.size(); ++i)
		mapSemanticService[i]->init();
}

void CSemanticJudge::runAnalysis(const char *szInput, JSONObject &jsonResp)
{
	unsigned int i;
	int nScore;
	map<string, string> mapMatch;
	CRankingHandler<int, int> ranking;
	CResponsePacket respPacket;
	CDisplayHandler display;

	if(0 >= szInput)
	{
		respPacket.setActivity<int>("type", RESP_TTS).setActivity<const char*>("lang", "zh").setActivity<const char*>(
				"tts",
				WORD_UNKNOW).setDisplay(display.getSadDisplay().c_str()).format(jsonResp);
		return;
	}

	for(i = 1; i <= mapAnalysis.size(); ++i)
	{
		nScore = mapAnalysis[i]->evaluate(szInput, mapMatch);
		ranking.add(i, nScore);
		_log("[CSemanticJudge] runAnalysis - %s Get Score: %d", mapAnalysis[i]->getName().c_str(), nScore);
	}

	//======== 評估特殊關聯服務字詞 ===========//
	if(mapMatch["dictionary"].empty())
	{
		for(i = 0; i < mapSemanticService.size(); ++i)
		{
			_log("[CSemanticJudge] runAnalysis 評估特殊關聯服務字詞 service: %s", mapSemanticService[i]->name().getBuffer());
			if(mapSemanticService[i]->evaluate(szInput, mapMatch))
			{
				for(unsigned int j = 1; j <= mapAnalysis.size(); ++j)
				{
					nScore = mapAnalysis[j]->evaluate(szInput, mapMatch);
					ranking.add(j, nScore);
					_log("[CSemanticJudge] runAnalysis - %s Get Score: %d Match: %s", mapAnalysis[j]->getName().c_str(),
							nScore, mapMatch["dictionary"].c_str());
				}
			}
		}
	}

	//============== 積分比較 ================//
	if(0 < ranking.topValue())
	{
		_log("[CSemanticJudge] word Top Key is %s", mapAnalysis[ranking.topValueKey()]->getName().c_str());
		mapAnalysis[ranking.topValueKey()]->activity(szInput, jsonResp, mapMatch);
	}
	else
	{
		respPacket.setActivity<int>("type", RESP_TTS).setActivity<const char*>("lang", "zh").setActivity<const char*>(
				"tts",
				WORD_UNKNOW).setDisplay(display.getSadDisplay().c_str()).format(jsonResp);
	}
}

void CSemanticJudge::runAnalysis(const char *szInput, JSONObject &jsonResp, const char *szAnalysis)
{
	map<string, string> mapMatch;
	CResponsePacket respPacket;
	CDisplayHandler display;
	string strDisplay;

	if(0 >= szInput)
	{
		respPacket.setActivity<int>("type", RESP_TTS).setActivity<const char*>("lang", "zh").setActivity<const char*>(
				"tts",
				WORD_UNKNOW).setDisplay(display.getSadDisplay().c_str()).format(jsonResp);
		return;
	}

	for(unsigned int i = 1; i <= mapAnalysis.size(); ++i)
	{
		if(!mapAnalysis[i]->getName().compare(szAnalysis))
		{
			_log("[CSemanticJudge] runAnalysis analysis: %s", szAnalysis);
			if(mapAnalysis[i]->evaluate(szInput, mapMatch) && mapMatch.size())
			{
				mapAnalysis[i]->activity(szInput, jsonResp, mapMatch);
				return;
			}

			//======== 評估特殊關聯服務字詞 ===========//
			if(mapMatch["dictionary"].empty())
			{
				for(unsigned int j = 0; j < mapSemanticService.size(); ++j)
				{
					_log("[CAnalysisHandler] evaluate 評估特殊關聯服務字詞 service: %s",
							mapSemanticService[j]->name().getBuffer());
					if(mapSemanticService[j]->evaluate(szInput, mapMatch))
					{
						mapAnalysis[i]->activity(szInput, jsonResp, mapMatch);
						return;
					}
				}

				//======== 歷史播放 ================//
			}
		}
	}

	respPacket.setActivity<int>("type", RESP_TTS).setActivity<const char*>("lang", "zh").setActivity<const char*>("tts",
	WORD_UNKNOW).setDisplay(display.getSadDisplay().c_str()).format(jsonResp);
}

void CSemanticJudge::onHandleMessage(Message &message)
{

}
