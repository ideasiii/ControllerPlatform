/*
 * CHttpsClient.cpp
 *
 *  Created on: 2017年4月25日
 *      Author: Jugo
 */

#include <string.h>
#include <string>
#include <curl/curl.h>
#include "CHttpsClient.h"
#include "LogHandler.h"

using namespace std;

static bool doneCurlInit = false;

static bool globalInit()
{
	printf("[CHttpsClient] curl_global_init(CURL_GLOBAL_ALL)\n");
	curl_global_init(CURL_GLOBAL_ALL);
	doneCurlInit = true;

	return true;
}

__attribute__ ((unused))__attribute__((destructor)) static void globalCleanup()
{
	if(doneCurlInit)
	{
		printf("[CHttpsClient] curl_global_cleanup()\n");
		curl_global_cleanup();
	}
}

CHttpsClient::CHttpsClient()
{
	static bool onlyOnce = globalInit();
}

CHttpsClient::~CHttpsClient()
{

}

size_t writeCallback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	// size*nmemb is the size of the buffer
	string *data = reinterpret_cast<string *>(userdata);

	for(unsigned int c = 0; c < size * nmemb; c++)
	{
		data->push_back(ptr[c]);
	}
	return size * nmemb; //tell curl how many bytes we handled
}

int CHttpsClient::GET(const char *szURL, string &strData, const set<string> &setHead)
{
	CURL *curl;
	CURLcode res;
	struct curl_slist *chunk = NULL;
	string data;

	curl = curl_easy_init();

	if(curl)
	{
		if(setHead.size())
		{
			for(set<string>::const_iterator it = setHead.begin(); setHead.end() != it; ++it)
			{
				chunk = curl_slist_append(chunk, it->c_str());
			}
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
			_log("[CHttpsClient] GET HEADER: %s", chunk->data);
		}

		_log("[CHttpsClient] GET URL: %s", szURL);

		curl_easy_setopt(curl, CURLOPT_URL, szURL);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writeCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
		//curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
		//curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);

		res = curl_easy_perform(curl);
		if(res != CURLE_OK)
			fprintf(stderr, "[CHttpsClient] GET curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

		curl_easy_cleanup(curl);
		if(NULL != chunk)
		{
			curl_slist_free_all(chunk);
		}

		strData = data;
	}

	return 1;
}

int CHttpsClient::POST(const char *szURL, std::string &strData, const std::set<std::string> &setHead,
		const std::set<std::string> &setParameter)
{
	CURL *curl;
	CURLcode res;
	struct curl_slist *chunk = NULL;
	string data;
	string strParameter;
	set<string>::const_iterator it;

	curl = curl_easy_init();

	if(curl)
	{
		if(setHead.size())
		{
			for(it = setHead.begin(); setHead.end() != it; ++it)
			{
				chunk = curl_slist_append(chunk, it->c_str());
			}
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
			_log("[CHttpsClient] POST HEADER: %s", chunk->data);
		}

		if(setParameter.size())
		{
			for(it = setParameter.begin(); setParameter.end() != it; ++it)
			{
				if(setParameter.begin() == it)
				{
					strParameter = *it;
				}
				else
				{
					strParameter += "&";
					strParameter += *it;
				}
			}
			_log("[CHttpsClient] POST Parameter: %s", strParameter.c_str());
		}

		_log("[CHttpsClient] POST URL: %s", szURL);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strParameter.c_str());
		curl_easy_setopt(curl, CURLOPT_URL, szURL);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writeCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
		//curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
		//curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);

		res = curl_easy_perform(curl);
		if(res != CURLE_OK)
			fprintf(stderr, "[CHttpsClient] POST curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

		curl_easy_cleanup(curl);
		if(NULL != chunk)
		{
			curl_slist_free_all(chunk);
		}

		strData = data;
	}

	return 0;
}

string urlEncode(const char *szStr)
{
	string strResult;
	CURL *curl = curl_easy_init();
	if(curl)
	{
		char *output = curl_easy_escape(curl, szStr, strlen(szStr));
		if(output)
		{
			strResult = output;
			curl_free(output);
		}

		curl_easy_cleanup(curl);
	}

	return strResult;
}
