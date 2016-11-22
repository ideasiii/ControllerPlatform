/*
 * CRdmOperate.cpp
 *
 *  Created on: 2016年7月28日
 *      Author: Jugo
 */

#include "CRdmOperate.h"
#include "JSONObject.h"
#include "JSONArray.h"

static CRdmOperate * rdmOperate = 0;

CRdmOperate::CRdmOperate()
{

}

CRdmOperate::~CRdmOperate()
{

}

CRdmOperate * CRdmOperate::getInstance()
{
	if (0 == rdmOperate)
	{
		rdmOperate = new CRdmOperate();
	}
	return rdmOperate;
}

string CRdmOperate::getOperate(string strId)
{
	string strOperate;

	JSONObject jsonRoot;
	JSONObject jsonControl;

	jsonRoot.put("result", 0);
	jsonControl.put("count", 0);
	jsonRoot.put("control", jsonControl);

	strOperate = jsonRoot.toString();

	jsonRoot.release();
	return strOperate;
}

