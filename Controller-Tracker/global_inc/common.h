#pragma once

#include <stdio.h>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>
#include <vector>
static std::string G_LOG_PATH;



#ifdef __cplusplus
extern "C"
{

#ifndef NULL
#define NULL	0
#endif

#ifndef BOOL
#define BOOL	int
#endif

#ifndef TRUE
#define TRUE	1
#endif

#ifndef FALSE
#define FALSE	0
#endif

#ifndef ULONG
#define ULONG	unsigned long
#endif

#ifndef SUCCESS
#define SUCCESS		1
#endif

#ifndef FAIL
#define	 FAIL				-1
#endif

#ifndef INVALID
#define INVALID			-1
#endif

#define _DBG(msg, arg...) printf("[DEBUG] " msg"\n" , ##arg)

#define _TRACE(msg, arg...) printf("[DEBUG] %s:%s(%d): " msg"\n" ,__FILE__, __FUNCTION__, __LINE__, ##arg)

#ifdef DUMPLOG
#define _DUMP(msg) \
{ \
		FILE *pstream; \
		pstream = fopen("/tmp/controller_client.log", "a"); \
		if(NULL != pstream) \
		{ \
			fprintf(pstream, "%s\n", msg); \
			fflush(pstream); \
			fclose(pstream); \
			system("sync;sync;sync"); \
		} \
}
#else
#define _DUMP(msg)
#endif

	__attribute__ ((unused)) static bool isValidStr(const char *szStr, int nMaxSize)
	{
		if ( (0 != szStr) && 0 < ((int) strlen( szStr )) && nMaxSize > ((int) strlen( szStr )) )
			return true;
		else
			return false;
	}

#define BUF_SIZE		2048	// socket send & recv buffer
#define BACKLOG		128		// How many pending connections queue will hold

}
#endif
