#pragma once

#include "WordInfo.h"
#include "dataType.h"

class CWord
{

public:
	explicit CWord();
	~CWord();

	void InitWord(LPCTSTR dir);
	void GetWord(WORD_PACKAGE &wordPackage);
	unsigned GetPhone(int ptr, char* txt);
	int IsNumberic(unsigned char *ch);
	void SetTone(int word_ndx, int char_ndx, USHORT new_tone, WORD_PACKAGE &wordPackage);
};
