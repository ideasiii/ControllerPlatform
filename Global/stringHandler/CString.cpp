/*
 * CString.cpp
 *
 *  Created on: 2017年9月30日
 *      Author: jugo
 */

#include <malloc.h>
#include <locale.h>
#include <stdio.h>
#include "string.h"
#include <assert.h>
#include <malloc.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>      // std::stringstream, std::stringbuf
#include <string.h>

#include "CString.h"

using namespace std;

CString::CString()
{
	m_pchData = NULL;
	m_iLen = 0;
	*this = "";
}

CString::~CString()
{
	if(m_pchData)
		free(m_pchData);
}

CString::CString(const CString& strSrc)
{
	m_pchData = NULL;
	m_iLen = 0;
	*this = strSrc;
}

CString::CString(TCHAR ch, int nRepeat)
{
	m_pchData = NULL;
	m_iLen = 0;
	*this = "";
	while(nRepeat--)
		*this += ch;
}

CString::CString(LPCSTR lpsz)
{
	m_pchData = NULL;
	m_iLen = 0;
	*this = lpsz;
}

CString::CString(const unsigned char* psz)
{
	m_pchData = NULL;
	m_iLen = 0;
	*this = psz;
}

int CString::getLength() const
{
	if(!m_pchData)
		return 0;
	return m_iLen;
}

bool CString::isEmpty()
{
	return (!m_pchData || !m_iLen);
}

void CString::empty()
{
	*this = "";
}

TCHAR CString::getAt(int nIndex)
{
	if((nIndex >= 0) && (nIndex < getLength()))
		return m_pchData[nIndex];
	else
		return 0;
}

TCHAR CString::operator[](int nIndex)
{
	return getAt(nIndex);
}

void CString::setAt(int nIndex, TCHAR ch)
{
	if((nIndex >= 0) && (nIndex < getLength()))
		m_pchData[nIndex] = ch;
}

const CString& CString::operator=(const CString& strSrc)
{
	if(&strSrc == NULL)
		return *this;
	int nSrcLen = ((CString&) strSrc).getLength();
	setAllocSize(nSrcLen + 1);
	copyMem(((CString&) strSrc).getBuffer(), nSrcLen + 1);
	setLength(nSrcLen);
	return *this;
}

const CString& CString::operator=(TCHAR ch)
{
	int nSrcLen = sizeof(TCHAR);
	setAllocSize(nSrcLen + 1);
	copyMem(&ch, nSrcLen);
	setLength(nSrcLen);
	return *this;
}

const CString& CString::operator=(LPCSTR lpsz)
{
	if(!lpsz)
		return *this;
	int nSrcLen = strlen(lpsz);
	setAllocSize(nSrcLen + 1);
	copyMem((void *) lpsz, nSrcLen + 1);
	setLength(nSrcLen);
	return *this;
}

const CString& CString::operator=(const unsigned char* psz)
{
	if(!psz)
		return *this;
	int nSrcLen = strlen((char *) psz);
	setAllocSize(nSrcLen + 1);
	copyMem((void *) psz, nSrcLen + 1);
	setLength(nSrcLen);
	return *this;
}

LPTSTR CString::getBuffer(int nMinBufLength)
{
	if(nMinBufLength != -1)
		setAllocSize(nMinBufLength);

	return m_pchData;
}

void CString::releaseBuffer(int nNewLength)
{
	if(nNewLength != -1)
		setLength(nNewLength);
	else
		setLength(strlen(m_pchData));
	setAllocSize(getLength() + 1);
}

// private functions...

int CString::getAllocSize()
{
	if(!m_pchData)
		return 0;
	return malloc_usable_size(m_pchData);
}

void CString::setAllocSize(int nSize)
{
	if(!m_pchData)
	{
		m_pchData = (char *) malloc(nSize);
		return;
	}

	if(getAllocSize() != nSize)
	{
		m_pchData = (char *) realloc(m_pchData, nSize);
	}
}

void CString::copyMem(void *pBuffer, int nLength)
{
	if(!pBuffer)
		return;
	if(!m_pchData)
		return;
	if(nLength <= 0)
		return;

	setAllocSize(nLength + 1);

	memcpy(m_pchData, pBuffer, nLength);
}

void CString::appendMem(void *pBuffer, int nLength)
{
	if(!pBuffer)
		return;
	if(!m_pchData)
		return;
	if(nLength <= 0)
		return;

	setAllocSize(getAllocSize() + nLength);

	memcpy(m_pchData + getLength(), pBuffer, nLength);
}

void CString::setLength(int nLength)
{
	if(!m_pchData)
		return;
	m_pchData[nLength] = 0;
	setAllocSize(nLength + 1);
	m_iLen = nLength;
}

