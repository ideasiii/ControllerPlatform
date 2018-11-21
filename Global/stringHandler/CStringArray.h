/*
 * CStringArray.h
 *
 *  Created on: 2018年10月22日
 *      Author: jugo
 */

#pragma once

#include "CString.h"
#include <vector>

class CStringArray
{
public:
	CStringArray();
	virtual ~CStringArray();
	int getSize();
	int add(const CString& newElement);
	void removeAll();
	// return single CString Referenceat zero-based index
	CString& operator[](int nIndex);

private:
	std::vector<CString> vString;

};
