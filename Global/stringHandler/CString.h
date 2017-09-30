/*
 * CString.h
 *
 *  Created on: 2017年9月30日
 *      Author: jugo
 */

#pragma once

#ifdef _UNICODE
typedef wchar_t TCHAR;
#else
typedef char TCHAR;
#endif

typedef TCHAR* LPTSTR;				// char* or wchar_t* depending on _UNICODE
typedef const TCHAR* LPCTSTR;		// const char* or const wchar_t* depending on _UNICODE
typedef char* LPSTR;
typedef const char* LPCSTR;
typedef wchar_t* LPWSTR;

class CString
{
public:
	// constructs empty CString
	CString();
	// copy constructor
	CString(const CString& stringSrc);
	// from a single character
	CString(TCHAR ch, int nRepeat = 1);
	// from ANSI string
	CString(LPCSTR lpsz);
	// from unsigned char's
	CString(const unsigned char* psz);

	virtual ~CString();

	// get data length
	int getLength() const;
	bool isEmpty();
	// clear contents to empty
	void empty();
	// return single character at zero-based index
	TCHAR getAt(int nIndex);
	// return single character at zero-based index
	TCHAR operator[](int nIndex);
	// set a single character at zero-based index
	void setAt(int nIndex, TCHAR ch);

	// Assignment

	// copy from another CString
	const CString& operator=(const CString& strSrc);
	// set string content to single character
	const CString& operator=(TCHAR ch);
	// copy string content from ANSI string (converts to TCHAR)
	const CString& operator=(LPCSTR lpsz);
	// copy string content from unsigned chars
	const CString& operator=(const unsigned char* psz);

	// get pointer to modifiable buffer at least as long as nMinBufLength (or as string length if -1)
	LPTSTR getBuffer(int nMinBufLength = -1);
	// release buffer, setting length to nNewLength (or to first nul if -1)
	void releaseBuffer(int nNewLength = -1);
protected:
	LPTSTR m_pchData;   // pointer to ref counted string data
	int m_iLen;

private:
	// get amount of allocated memory
	int getAllocSize();
	// alloc's/frees memory if needed
	void setAllocSize(int nSize);
	// copies bytes
	void copyMem(void *pBuffer, int nLength);
	// append-copies bytes
	void appendMem(void *pBuffer, int nLength);
	// sets terminating NULL
	void setLength(int nLength);

};
