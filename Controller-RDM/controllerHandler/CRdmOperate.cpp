/*
 * CRdmOperate.cpp
 *
 *  Created on: 2016年7月28日
 *      Author: Jugo
 */

#include "CRdmOperate.h"

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

