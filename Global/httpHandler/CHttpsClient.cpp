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

int CHttpsClient::GET(const char *szURL, string &strData)
{
	CURL *curl;
	CURLcode res;
	data.clear();

	curl_global_init(CURL_GLOBAL_DEFAULT);

	curl = curl_easy_init();
	if(curl)
	{
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

/*
 string url_encode(const string &value)
 {
 ostringstream escaped;
 escaped.fill('0');
 escaped << hex;

 for(string::const_iterator i = value.begin(), n = value.end(); i != n; ++i)
 {
 string::value_type c = (*i);

 // Keep alphanumeric and other accepted characters intact
 if(isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
 {
 escaped << c;
 continue;
 }

 // Any other characters are percent-encoded
 escaped << uppercase;
 escaped << '%' << setw(2) << int((unsigned char) c);
 escaped << nouppercase;
 }

 return escaped.str();
 }
 */
