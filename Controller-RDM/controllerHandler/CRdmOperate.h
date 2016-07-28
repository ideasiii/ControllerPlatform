/*
 * CRdmOperate.h
 *
 *  Created on: 2016年7月28日
 *      Author: Jugo
 */

#pragma once

class CRdmOperate
{
public:
	static CRdmOperate * getInstance();
	virtual ~CRdmOperate();

private:
	explicit CRdmOperate();
};
