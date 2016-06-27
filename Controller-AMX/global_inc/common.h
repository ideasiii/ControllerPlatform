#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>

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

#ifndef UINT
#define UINT unsigned int
#endif

#define _DBG(msg, arg...) printf("[DEBUG] " msg"\n" , ##arg)
#define _TRACE(msg, arg...) printf("[DEBUG] %s:%s(%d): " msg"\n" ,__FILE__, __FUNCTION__, __LINE__, ##arg)

#define BUF_SIZE		2048	// socket send & recv buffer
#define BACKLOG		128		// How many pending connections queue will hold
#define SUCCESS		1
#define	 FAIL				-1
};
#endif
