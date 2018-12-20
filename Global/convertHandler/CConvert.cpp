/*
 * CConvert.cpp
 *
 *  Created on: 2018年11月26日
 *      Author: jugo
 */

#include "iconv.h"
#include "string.h"
#include "stdlib.h"
#include "errno.h"
#include "CConvert.h"
#include "LogHandler.h"

CConvert::CConvert()
{

}

CConvert::~CConvert()
{

}

int CConvert::UTF8toBig5(char *szFrom, char **szTo)
{
	iconv_t cd;
	size_t in_s, out_s;

	/* UTF-8 轉 Big5 */
	cd = iconv_open("BIG-5", "UTF8");

	char *ibuf = const_cast<char*>(szFrom), *in_ptr, *out_ptr;
	*szTo = NULL;

	in_s = strlen(ibuf);
	in_ptr = ibuf;

	*szTo = (char*) malloc(in_s * 3);
	out_s = in_s * 3;
	out_ptr = *szTo;

	if (cd == (iconv_t) -1)
	{
		_log("[CConvert] UTF8toBig5 error opening iconv");
		free(*szTo);
		return -1;
	}

	if ((int) iconv(cd, &in_ptr, &in_s, &out_ptr, &out_s) == -1)
	{
		_log("[CConvert] UTF8toBig5 errno: %s\n", strerror(errno));
		free(*szTo);
		return -1;
	}

	*out_ptr = '\0';

	iconv_close(cd);

	//_log("[CConvert] UTF8toBig5 Finish: %s --> %s", ibuf, *szTo);
	return 0;
}

int CConvert::Big5toUTF8(char *szFrom, char **szTo)
{
	iconv_t cd;
	size_t in_s, out_s;

	/*   Big5 轉 UTF-8*/
	cd = iconv_open("UTF8", "BIG-5");

	char *ibuf = const_cast<char*>(szFrom), *in_ptr, *out_ptr;
	*szTo = NULL;

	in_s = strlen(ibuf);
	in_ptr = ibuf;

	*szTo = (char*) malloc(in_s * 3);
	out_s = in_s * 3;
	out_ptr = *szTo;

	if (cd == (iconv_t) -1)
	{
		_log("[CConvert] Big5toUTF8 error opening iconv");
		free(*szTo);
		return -1;
	}

	if ((int) iconv(cd, &in_ptr, &in_s, &out_ptr, &out_s) == -1)
	{
		_log("[CConvert] Big5toUTF8 errno: %s  %s --> %s", strerror(errno), ibuf, *szTo);
		free(*szTo);
		iconv_close(cd);
		return -1;
	}

	*out_ptr = '\0';

	iconv_close(cd);

	//_log("[CConvert] Big5toUTF8 Finish: %s --> %s", ibuf, *szTo);
	return 0;
}

int CConvert::Big5toUTF8(char *szFrom, size_t nFrom, char **szTo)
{
	iconv_t cd;
	size_t in_s, out_s;

	/*   Big5 轉 UTF-8*/
	cd = iconv_open("UTF8", "BIG-5");

	char *ibuf = const_cast<char*>(szFrom), *in_ptr, *out_ptr;
	*szTo = NULL;

	in_s = nFrom;
	in_ptr = ibuf;

	*szTo = (char*) malloc(in_s * 3);
	out_s = in_s * 3;
	out_ptr = *szTo;

	if (cd == (iconv_t) -1)
	{
		_log("[CConvert] Big5toUTF8 error opening iconv");
		free(*szTo);
		return -1;
	}

	if ((int) iconv(cd, &in_ptr, &in_s, &out_ptr, &out_s) == -1)
	{
		_log("[CConvert] Big5toUTF8 errno: %s\n", strerror(errno));
		free(*szTo);
		return -1;
	}

	*out_ptr = '\0';

	iconv_close(cd);

	//_log("[CConvert] Big5toUTF8 Finish: %s --> %s", ibuf, *szTo);
	return 0;
}

