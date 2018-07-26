/*
 * CParticiple.cpp
 *
 *  Created on: 2018年7月26日
 *      Author: Jugo
 */

#include "CParticiple.h"
#include "CString.h"
#include "common.h"

using namespace std;

CParticiple::CParticiple()
{
	// TODO Auto-generated constructor stub

}

CParticiple::~CParticiple()
{
	// TODO Auto-generated destructor stub
}

void CParticiple::splitter(const char *szContent, set<string> splitterMark)
{
	CString strContent;

	if(!szContent || splitterMark.empty())
		return;
	strContent = szContent;

	_log("[CParticiple] splitter Content: %s", szContent);
}

