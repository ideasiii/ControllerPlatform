/*
 * CStringArray.cpp
 *
 *  Created on: 2018年10月22日
 *      Author: jugo
 */

#include "CStringArray.h"

using namespace std;

CStringArray::CStringArray()
{

}

CStringArray::~CStringArray()
{

}

int CStringArray::getSize()
{
	return vString.size();
}

int CStringArray::add(const CString& newElement)
{
	vString.push_back(newElement);
	return (int) vString.size();
}

CString& CStringArray::operator[](int nIndex)
{
	return vString[nIndex];
}

void CStringArray::removeAll()
{
	vString.clear();
}

