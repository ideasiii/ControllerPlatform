/*
 * CHttpsClient.cpp
 *
 *  Created on: 2017年4月25日
 *      Author: Jugo
 */

#include <string.h>
#include <string>
#include <set>
#include <curl/curl.h>
#include "CHttpsClient.h"
#include "LogHandler.h"

using namespace std;

CHttpsClient::CHttpsClient()
{

}

CHttpsClient::~CHttpsClient()
{

}

string data; //will hold the url's contents

size_t writeCallback(char* buf, size_t size, size_t nmemb, void* up)
{ //callback must have this declaration
//buf is a pointer to the data that curl has for us
//size*nmemb is the size of the buffer

	for(unsigned int c = 0; c < size * nmemb; c++)
	{
		data.push_back(buf[c]);
	}
	return size * nmemb; //tell curl how many bytes we handled
}

int CHttpsClient::GET(const char *szURL, string &strData, set<string> &setHead)
{
	CURL *curl;
	CURLcode res;
	data.clear();
	struct curl_slist *chunk = NULL;

	curl_global_init(CURL_GLOBAL_DEFAULT);
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
		}

		_log("[CHttpsClient] GET URL: %s", szURL);
		curl_easy_setopt(curl, CURLOPT_URL, szURL);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writeCallback);
		/* Perform the request, res will get the return code */
		res = curl_easy_perform(curl);
		/* Check for errors */
		if(res != CURLE_OK)
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

		/* always cleanup */
		curl_easy_cleanup(curl);
		strData = data;
	}
	return 1;
}

int CHttpsClient::POST(const char *szURL, std::string &strData, std::set<std::string> &setHead,
		std::set<std::string> &setParameter)
{
	string strParameter;
	CURL *curl;
	CURLcode res;
	data.clear();
	struct curl_slist *chunk = NULL;
	set<string>::const_iterator it;

	curl_global_init(CURL_GLOBAL_ALL);
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
		}
		_log("[CHttpsClient] GET URL: %s", szURL);
		/* First set the URL that is about to receive our POST. This URL can
		 just as well be a https:// URL if that is what should receive the
		 data. */
		//curl_easy_setopt(curl, CURLOPT_URL, "http://postit.example.com/moo.cgi");
		/* Now specify the POST data */
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strParameter.c_str());
		curl_easy_setopt(curl, CURLOPT_URL, szURL);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writeCallback);

		/* Perform the request, res will get the return code */
		res = curl_easy_perform(curl);
		/* Check for errors */
		if(res != CURLE_OK)
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

		/* always cleanup */
		curl_easy_cleanup(curl);
		strData = data;
	}
	curl_global_cleanup();
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
	}
	return strResult;
}
