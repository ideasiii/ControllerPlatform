/*
 * CTranslate.cpp
 *
 *  Created on: 2017年6月20日
 *      Author: root
 */

#include <set>
#include <string>
#include <string.h>
#include <openssl/md5.h>
#include "CTranslate.h"
#include "CHttpsClient.h"
#include "LogHandler.h"
#include "utility.h"
#include "JSONObject.h"
#include "JSONArray.h"

#define TRANS_HOST			"http://api.fanyi.baidu.com/api/trans/vip/translate"

using namespace std;

CTranslate::CTranslate()
{

}

CTranslate::~CTranslate()
{

}

int CTranslate::translate(LANGUAGE lang, const char *szInput, RESULT &result)
{
	string strData;
	set<string> setHead;
	set<string> setParameter;
	CHttpsClient httpsClient;
	JSONObject jroot;
	JSONObject jItem;
	JSONArray jArray;
	char sign[120];
	unsigned char md[16];
	int i;
	char tmp[3] = { '\0' }, buf[33] = { '\0' };

	if(!szInput)
		return -1;

	setParameter.insert(format("q=%s", szInput));
	setParameter.insert("from=auto");
	setParameter.insert(format("to=%s", getLang(lang).c_str()));
	setParameter.insert("appid=20170620000059502");
	setParameter.insert("salt=666");

	memset(sign, 0, sizeof(sign));
	strcat(sign, "20170620000059502");
	strcat(sign, szInput);
	strcat(sign, "666");
	strcat(sign, "GkPzfNCZUCDq_xcLyICN");

	MD5((unsigned char *) sign, strlen((const char *) sign), md);
	for(i = 0; i < 16; i++)
	{
		sprintf(tmp, "%2.2x", md[i]);
		strcat(buf, tmp);
	}

	setParameter.insert(format("sign=%s", buf));

	httpsClient.POST(TRANS_HOST, strData, setHead, setParameter);
	_log("[CTranslate] translate Response Data: %s", strData.c_str());

	if(jroot.load(strData).isValid())
	{
		jArray = jroot.getJsonArray("trans_result");
		if(jArray.isValid())
		{
			jItem = jArray.getJsonObject(0);
			if(jItem.isValid())
			{
				result.strResult = jItem.getString("dst");
			}
		}
	}
	jroot.release();
	return 0;
}

string CTranslate::getLang(LANGUAGE lang)
{
	string strLang;

	switch(lang)
	{
	case AUTO:
		strLang = "auto";
		break;
	case zh:			//中文
		strLang = "zh";
		break;
	case en:			//英语
		strLang = "en";
		break;
	case yue:			//粤语
		strLang = "yue";
		break;
	case wyw:			//文言文
		strLang = "wyw";
		break;
	case jp:			//日语
		strLang = "jp";
		break;
	case kor:			//韩语
		strLang = "kor";
		break;
	case fra:			//法语
		strLang = "fra";
		break;
	case spa:			//西班牙语
		strLang = "spa";
		break;
	case th:			//泰语
		strLang = "th";
		break;
	case ara:			//阿拉伯语
		strLang = "ara";
		break;
	case ru:			//俄语
		strLang = "ru";
		break;
	case pt:			//葡萄牙语
		strLang = "pt";
		break;
	case de:			//德语
		strLang = "de";
		break;
	case it:			//意大利语
		strLang = "it";
		break;
	case el:			//希腊语
		strLang = "el";
		break;
	case nl:			//荷兰语
		strLang = "nl";
		break;
	case pl:			//波兰语
		strLang = "pl";
		break;
	case bul:			//保加利亚语
		strLang = "bul";
		break;
	case est:			//爱沙尼亚语
		strLang = "est";
		break;
	case dan:			//丹麦语
		strLang = "dan";
		break;
	case fin:			//芬兰语
		strLang = "fin";
		break;
	case cs:			//捷克语
		strLang = "cs";
		break;
	case rom:			//罗马尼亚语
		strLang = "rom";
		break;
	case slo:			//斯洛文尼亚语
		strLang = "slo";
		break;
	case swe:			//瑞典语
		strLang = "swe";
		break;
	case hu:			//匈牙利语
		strLang = "hu";
		break;
	case cht:			//繁体中文
		strLang = "cht";
		break;
	case vie:			//越南语
		strLang = "vie";
		break;
	}

	return strLang;
}

