/*
 * CString.cpp
 *
 *  Created on: 2017年9月30日
 *      Author: jugo
 */

#include <malloc.h>
#include <locale.h>
#include <stdio.h>
#include <assert.h>
#include <malloc.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <string.h>
#include <cstdarg>

#include "CString.h"

using namespace std;

inline int memicmp(const void *s1, const void *s2, size_t n)
{
	size_t i;
	int d;

	for(i = 0; i < n; ++i)
	{
		d = tolower(((unsigned char *) s1)[i]) - tolower(((unsigned char *) s2)[i]);
		if(d != 0)
		{
			if(d > 0)
				return (1);
			else
				return (-1);
		}
	}
	return (0);
}

inline void _strupr(char *p)
{
	while(*p)
	{
		*p = toupper(*p);
		++p;
	}
}

inline void _strlwr(char* p)
{
	while(*p)
	{
		*p = tolower(*p);
		++p;
	}
}

inline void _strrev(char *str)
{

	int i = strlen(str) - 1, j = 0;

	char ch;
	while(i > j)
	{
		ch = str[i];
		str[i] = str[j];
		str[j] = ch;
		--i;
		++j;
	}
}

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

CString::CString(std::string strString)
{
	m_pchData = NULL;
	m_iLen = 0;
	*this = strString.c_str();
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

const CString& CString::operator=(std::string strString)
{
	if(strString.empty())
		return *this;
	int nSrcLen = strlen((char *) strString.c_str());
	setAllocSize(nSrcLen + 1);
	copyMem((void *) strString.c_str(), nSrcLen + 1);
	setLength(nSrcLen);
	return *this;
}

CString::operator LPCTSTR()
{
	return this->getBuffer();
}

const CString& CString::operator+=(const CString& string)
{
	if(&string == NULL)
		return *this;
	int nSrcLen = ((CString&) string).getLength();
	setAllocSize(getLength() + nSrcLen + 1);
	appendMem(((CString&) string).getBuffer(), nSrcLen + 1);
	setLength(getLength() + nSrcLen);
	return *this;
}

const CString& CString::operator+=(TCHAR ch)
{
	int nSrcLen = sizeof(TCHAR);
	setAllocSize(getAllocSize() + 1);
	appendMem(&ch, nSrcLen);
	setLength(getLength() + nSrcLen);
	return *this;
}

const CString& CString::operator+=(const unsigned char* psz)
{
	if(!psz)
		return *this;
	int nSrcLen = strlen((char *) psz);
	setAllocSize(getLength() + nSrcLen + 1);
	appendMem((void *) psz, nSrcLen + 1);
	setLength(getLength() + nSrcLen);
	return *this;
}

CString operator+(const CString& string1, const CString& string2)
{
	CString s;
	s = string1;
	s += string2;
	return s;
}

CString operator+(const CString& string, TCHAR ch)
{
	CString s;
	s = string;
	s += ch;
	return s;
}

CString operator+(TCHAR ch, const CString& string)
{
	CString s;
	s = ch;
	s += string;
	return s;
}

CString operator+(const CString& string, LPCTSTR lpsz)
{
	CString s;
	s = string;
	s += lpsz;
	return s;
}

CString operator+(LPCTSTR lpsz, const CString& string)
{
	CString s;
	s = lpsz;
	s += string;
	return s;
}

int CString::Compare(LPCTSTR lpsz)
{
	if(!lpsz)
		return 0;
//	setlocale(LC_ALL, "");

	int iLenMe = getLength();
	int iLenYou = strlen(lpsz);
	int iLenComp = (iLenMe < iLenYou) ? iLenMe : iLenYou;
	int iResult = memcmp(m_pchData, lpsz, iLenComp);
	if(iResult)
		return iResult;
	else
	{
		if(iLenMe == iLenYou)
			return 0;
		if(iLenMe < iLenYou)
			return -1;
		if(iLenMe > iLenYou)
			return 1;
	}
	return 0;  // never reached but to avoid c++ warning
}

int CString::CompareNoCase(LPCTSTR lpsz)
{
	if(!lpsz)
		return 0;
//	setlocale(LC_ALL, "");

	int iLenMe = getLength();
	int iLenYou = strlen(lpsz);
	int iLenComp = (iLenMe < iLenYou) ? iLenMe : iLenYou;
	int iResult = memicmp(m_pchData, lpsz, iLenComp);
	if(iResult)
		return iResult;
	else
	{
		if(iLenMe == iLenYou)
			return 0;
		if(iLenMe < iLenYou)
			return -1;
		if(iLenMe > iLenYou)
			return 1;
	}
	return 0;  // never reached but to avoid c++ warning
}

bool CString::operator==(const char* psz)
{
	return !(this->Compare(psz));
}

bool CString::operator!=(const char* psz)
{
	return (this->Compare(psz));
}

CString CString::mid(int nFirst, int nCount)
{
	// out-of-bounds requests return sensible things
	if(nFirst < 0)
		nFirst = 0;
	if(nCount < 0)
		nCount = 0;

	if(nFirst + nCount > getLength())
		nCount = getLength() - nFirst;
	if(nFirst > getLength())
		nCount = 0;

	// optimize case of returning entire string
	if(nFirst == 0 && nFirst + nCount == getLength())
		return *this;

	CString dest;
	dest = (getBuffer() + nFirst);
	dest.setLength(nCount);

	return dest;
}

CString CString::mid(int nFirst)
{
	return mid(nFirst, getLength() - nFirst);
}

CString CString::left(int nCount)
{
	return mid(0, nCount);
}

CString CString::right(int nCount)
{
	return mid(getLength() - nCount, nCount);
}

CString CString::part(char cDelimiter, int nIndex)
{
	if(this->find(cDelimiter, 0) == -1)
		return *this;

	int iPos = 0;
	int iEnd;
	CString str;

	while(nIndex--)
	{
		iPos = this->find(cDelimiter, iPos) + 1;
		if(!iPos)
			return str;
	}

	iEnd = this->find(cDelimiter, iPos);
	if(iEnd == -1)
		iEnd = getLength();

	str = mid(iPos, iEnd - iPos);

	return str;
}

int CString::partCount(char cDelimiter)
{
	if(isEmpty())
		return 0;
	CString str = *this;
	return str.remove(cDelimiter) + 1;
}

int CString::partBegin(char cDelimiter, int nIndex)
{
	if(find(cDelimiter, 0) == -1)
		return 0;
	if(isEmpty())
		return 0;
	if(nIndex == 0)
		return 0;

	int iPos = 0;
	while(nIndex--)
	{
		iPos = find(cDelimiter, iPos) + 1;
		if(!iPos)
			return iPos;
	}

	return iPos;
}

CString& CString::makeUpper()
{
//	setlocale(LC_ALL, "");
	if(m_pchData)
		_strupr(m_pchData);
	return *this;
}

CString& CString::makeLower()
{
//	setlocale(LC_ALL, "");
	if(m_pchData)
		_strlwr(m_pchData);
	return *this;
}

CString& CString::makeReverse()
{
//	setlocale(LC_ALL, "");
	if(m_pchData)
		_strrev(m_pchData);
	return *this;
}

CString& CString::trimRight()
{
	// check chars from the end on for space
	int iPos = getLength() - 1;
	while(isspace(getAt(iPos)) && (iPos >= 0))
		--iPos;
	++iPos;
	setLength(iPos);
	return *this;
}

CString& CString::trimLeft()
{
	// check chars from the beginning on for space
	int iPos = 0;
	int iLen = getLength();
	while(isspace(getAt(iPos)) && (iPos < iLen))
		++iPos;
	*this = mid(iPos);
	return *this;
}

CString& CString::trim()
{
	trimLeft();
	trimRight();
	return *this;
}

int CString::replace(TCHAR chOld, TCHAR chNew)
{
	// check chars from the beginning on
	int iPos;
	int iLen = getLength();
	int iCount = 0;
	for(iPos = 0; iPos < iLen; iPos++)
	{
		if(getAt(iPos) == chOld)
		{
			setAt(iPos, chNew);
			iCount++;
		}
	}
	return iCount;
}

int CString::replace(LPCTSTR lpszOld, LPCTSTR lpszNew)
{
	if(!lpszOld)
		return -1;
	if(!lpszNew)
		return -1;

	// check string from the beginning on
	int iPos;
	int iLenOld = strlen(lpszOld);
	int iLen = getLength() - iLenOld;
	int iCount = 0;
	for(iPos = 0; iPos < iLen; iPos++)
	{
		if(mid(iPos, iLenOld) == lpszOld)
		{
			Delete(iPos, iLenOld);
			insert(iPos, lpszNew);
			iCount++;
		}
	}
	return iCount;
}

int CString::remove(TCHAR chRemove)
{
	// check chars from the beginning on
	int iPos;
	int iLen = getLength();
	int iCount = 0;
	for(iPos = 0; iPos < iLen; iPos++)
	{
		if(getAt(iPos) == chRemove)
		{
			Delete(iPos);
			iCount++;
		}
	}
	return iCount;
}

int CString::insert(int nIndex, TCHAR ch)
{
	if(nIndex < 0)
		nIndex = 0;
	if(nIndex > getLength())
		nIndex = getLength();

	setAllocSize(getAllocSize() + 1);

	// move anything right of new char to right
	int iPos;
	int iLen = getLength();
	for(iPos = iLen; iPos >= nIndex; --iPos)
		setAt(iPos + 1, getAt(iPos));
	setLength(++iLen);
	setAt(nIndex, ch);

	return iLen;
}

int CString::insert(int nIndex, LPCTSTR pstr)
{
	if(!pstr)
		return -1;
	if(nIndex < 0)
		nIndex = 0;
	if(nIndex > getLength())
		nIndex = getLength();

	// TO-DO: do this like in Delete()
	// insert new string char by char
	int iPos;
	int iLen = strlen(pstr);
	for(iPos = 0; iPos < iLen; ++iPos)
		insert(nIndex + iPos, pstr[iPos]);

	return getLength();
}

int CString::Delete(int nIndex, int nCount)
{
	if(nIndex < 0)
		nIndex = 0;
	if(nIndex > getLength())
		nIndex = getLength();

	// move anything right of nIndex to left
	int iPos;
	int iLen = getLength();
	for(iPos = nIndex; iPos + nCount <= iLen; ++iPos)
		setAt(iPos, getAt(iPos + nCount));
	setLength(iLen -= nCount);

	return iLen;
}

int CString::find(TCHAR ch)
{
	return find(ch, 0);
}

int CString::reverseFind(TCHAR ch)
{
	int iPos;
	int iLen = getLength();
	for(iPos = iLen; iPos >= 0; iPos--)
	{
		if(getAt(iPos) == ch)
			return iPos;
	}
	return -1;
}

int CString::find(TCHAR ch, int nStart)
{
	int iPos;
	int iLen = getLength();
	for(iPos = nStart; iPos < iLen; ++iPos)
	{
		if(getAt(iPos) == ch)
			return iPos;
	}
	return -1;
}

int CString::find(LPCTSTR lpszSub)
{
	return find(lpszSub, 0);
}

int CString::find(LPCTSTR lpszSub, int nStart)
{
	int nIndex;
	strData = m_pchData;
	if(!strData.empty() && lpszSub)
	{
		nIndex = strData.find(lpszSub, nStart);
		if((int) string::npos != nIndex)
		{
			return nIndex;
		}
	}
	return -1;
}

void CString::format(const char *pcFormat, ...)
{
	va_list vl;
	va_start(vl, pcFormat);
	int size = vsnprintf(0, 0, pcFormat, vl) + sizeof('\0');
	va_end(vl);

	setAllocSize(size);

	va_list argList;
	va_start(argList, pcFormat);
	vsprintf(m_pchData, pcFormat, argList);
	va_end(argList);

	releaseBuffer();
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

string CString::toString()
{
	strData = m_pchData;
	return strData;
}

