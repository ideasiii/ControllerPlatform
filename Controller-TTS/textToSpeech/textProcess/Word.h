#pragma once

#include "WordInfo.h"
#include <fstream>

extern char* word_attr[];

#define SENTENCE_LEN 100

enum
{
	CHINESE_CHAR, ENGLISH_CHAR, DIGIT_CHAR, SYMBOL_CHAR, DOT_CHAR, SPECIAL_DIGIT, SPECIAL_CHAR, MOUSE_CHAR
};

class CWord
{
public:
	// 以下這兩個變數是為了將斷詞系統所載入的詞庫輸出所設計
	WORD_DB *m_word_data;			// 在InitWord中，會將其指向word_data靜態變數
	unsigned short *m_word_index;	// 在InitWord中，會將其指向word_index靜態變數

public:
	CWord();
	~CWord();

	void InitWord(const char* dir);
	void GetWord();
	void SetCharType();
	unsigned GetPhone(int ptr);
	void Score(int cur_ptr);
	void ChangePhone();
	int ReadText(std::ofstream* fp);
	int GetSentence(unsigned char * from, int *textNdx);
	int IsNumberic(unsigned char *ch);
	void SetTone(int word_ndx, int char_ndx, unsigned short new_tone);
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
	bool m_init;
};
