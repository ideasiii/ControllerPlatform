#pragma once

#include "WordInfo.h"
#include "dataType.h"

//extern LPCTSTR word_attr[];

//enum
//{
//	CHINESE_CHAR, ENGLISH_CHAR, DIGIT_CHAR, SYMBOL_CHAR, DOT_CHAR, SPECIAL_DIGIT, SPECIAL_CHAR, MOUSE_CHAR
//};

class CWord
{

public:
	explicit CWord();
	~CWord();

	void InitWord(LPCTSTR dir);
	void GetWord(WORD_PACKAGE &wordPackage);
//	void SetCharType(WORD_PACKAGE &wordPackage);
	unsigned GetPhone(int ptr, WORD_PACKAGE &wordPackage);
	void Score(int cur_ptr, WORD_PACKAGE &wordPackage);
	void ChangePhone(WORD_PACKAGE &wordPackage);
	int GetSentence(UCHAR * from, int *textNdx, WORD_PACKAGE &wordPackage);
	int IsNumberic(unsigned char *ch);
	void SetTone(int word_ndx, int char_ndx, USHORT new_tone, WORD_PACKAGE &wordPackage);
};
