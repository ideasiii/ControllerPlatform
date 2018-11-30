#pragma once

#include "WordInfo.h"
#include <iostream>
#include <fstream>
#include "dataType.h"

extern LPCTSTR word_attr[];

#define SENTENCE_LEN 100

enum
{
	CHINESE_CHAR, ENGLISH_CHAR, DIGIT_CHAR, SYMBOL_CHAR, DOT_CHAR, SPECIAL_DIGIT, SPECIAL_CHAR, MOUSE_CHAR
};

class CWord
{
// 啟峻修改的部分
public:
	// 以下這兩個變數是為了將斷詞系統所載入的詞庫輸出所設計
	WORD_DB *m_word_data;			// 在InitWord中，會將其指向word_data靜態變數
	unsigned short *m_word_index;	// 在InitWord中，會將其指向word_index靜態變數
// 以下為陳昭宏學長原始的程式碼
public:
	CWord();
	~CWord();

	void InitWord(LPCTSTR dir);
	void GetWord();
	void SetCharType();
	unsigned GetPhone(int ptr);
	void Score(int cur_ptr);
	void ChangePhone();
	int ReadText(FILE* fp);
	int GetSentence(UCHAR * from, int *textNdx);
	int IsNumberic(unsigned char *ch);
	void SetTone(int word_ndx, int char_ndx, USHORT new_tone);
//	friend std::istream & operator >> (std::istream &in, WORD_DB* &obj);
public:
	int txt_len;
	int m_punctuation;
	unsigned char txt[SENTENCE_LEN * 2];	// 這個類別中所要處理的句子
	WORD_INFO w_info[80];				// 詞長，大五碼，發音代碼等等
	int wnum;							//	word num，也就是這個句子被斷成幾個詞

	int tab[SENTENCE_LEN][11];
	int ptrtab[SENTENCE_LEN][11];
	int best[SENTENCE_LEN];
	int q[SENTENCE_LEN * 2];
	int toneComb[SENTENCE_LEN * 2], toneComb4[SENTENCE_LEN * 2];	//toneComb與toneComb4的差別好像是，一個是五聲調，一個是四聲調

	int voicedType[SENTENCE_LEN * 2];			//有聲或無聲的子音
	int sentenceToneCobm[SENTENCE_LEN * 2];

	int best_score;
	int char_type[SENTENCE_LEN + 1];
	BOOL m_init;
};
