/*
 * CString.h
 *
 *  Created on: 2017年9月30日
 *      Author: jugo
 */

#pragma once

#include <string>

#ifdef _UNICODE
//typedef wchar_t TCHAR;
typedef char TCHAR;
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
	// from STL string
	CString(std::string strString);

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
	// copy string content from STL string
	const CString& operator=(std::string strString);

	// return pointer to const string
	operator LPCTSTR();

	// concatenate from another CString
	const CString& operator+=(const CString& string);
	// concatenate a single character
	const CString& operator+=(TCHAR ch);
	// concatenate unsigned chars
	const CString& operator+=(const unsigned char* psz);

	friend CString operator+(const CString& string1, const CString& string2);
	friend CString operator+(const CString& string, TCHAR ch);
	friend CString operator+(TCHAR ch, const CString& string);
	friend CString operator+(const CString& string, LPCTSTR lpsz);
	friend CString operator+(LPCTSTR lpsz, const CString& string);

	// String comparison

	// straight character comparison
	int Compare(LPCTSTR lpsz);
	// compare ignoring case
	int CompareNoCase(LPCTSTR lpsz);
	// compare unsigned chars using ==
	bool operator==(const char* psz);
	// compare unsigned chars using !=
	bool operator!=(const char* psz);

	// Sub-string extraction

	// return nCount characters starting at zero-based nFirst
	CString mid(int nFirst, int nCount);
	// return all characters starting at zero-based nFirst
	CString mid(int nFirst);
	// return first nCount characters in string
	CString left(int nCount);
	// return nCount characters from end of string
	CString right(int nCount);
	// return nth part
	CString part(char cDelimiter, int nIndex);
	// return number of parts
	int partCount(char cDelimiter);
	// return start position of nth part
	int partBegin(char cDelimiter, int nIndex);

	// upper/lower/reverse conversion

	// NLS aware conversion to uppercase
	CString& makeUpper();
	// NLS aware conversion to lowercase
	CString& makeLower();
	// reverse string right-to-left
	CString& makeReverse();

	// trimming whitespace (either side)

	// remove whitespace starting from right edge
	CString& trimRight();
	// remove whitespace starting from left side
	CString& trimLeft();
	// remove whitespace from any side
	CString& trim();

	// advanced manipulation

	// replace occurrences of chOld with chNew
	int replace(TCHAR chOld, TCHAR chNew);
	// replace occurrences of substring lpszOld with lpszNew;
	// empty lpszNew removes instances of lpszOld
	int replace(LPCTSTR lpszOld, LPCTSTR lpszNew);
	// remove occurrences of chRemove
	int remove(TCHAR chRemove);
	// insert character at zero-based index; concatenates
	// if index is past end of string
	int insert(int nIndex, TCHAR ch);
	// insert substring at zero-based index; concatenates
	// if index is past end of string
	int insert(int nIndex, LPCTSTR pstr);
	// delete nCount characters starting at zero-based index
	int Delete(int nIndex, int nCount = 1);

	// searching

	// find character starting at left, -1 if not found
	int find(TCHAR ch);
	// find character starting at right
	int reverseFind(TCHAR ch);
	// find character starting at zero-based index and going right
	int find(TCHAR ch, int nStart);
	// find first instance of substring
	int find(LPCTSTR lpszSub);
	// find first instance of substring starting at zero-based index
	int find(LPCTSTR lpszSub, int nStart);

	// formatting

	void format(const char *pcFormat, ...);

	// get pointer to modifiable buffer at least as long as nMinBufLength (or as string length if -1)
	LPTSTR getBuffer(int nMinBufLength = -1);
	// release buffer, setting length to nNewLength (or to first nul if -1)
	void releaseBuffer(int nNewLength = -1);

	// return STL string
	std::string toString();

protected:
	LPTSTR m_pchData;   // pointer to ref counted string data
	int m_iLen;
	std::string strData;

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
