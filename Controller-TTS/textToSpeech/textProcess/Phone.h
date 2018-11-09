#pragma once
#include "common.h"

typedef int (*CMPFUNC)(const void*, const void*);

#define	UNDEF   -1
#define	SNDID	short
#define NUM_PHONE		411
#define NUM_PINYIN		417
//定義中文字索引的數目，此值會略大於中文字的實際數目
//(0xF9-0xA4)*157 + 0x80 = 13345 + 128 = 13473
#define CHINESE_INDEX_NUM 13500

extern char *PHONE_TABLE[NUM_PHONE];

#define SD_BAD		0
#define SD_SYSTEM	4110
#define SD_DIGIT	4350
#define SD_PHONEME	4360
#define SD_ENG		4397
#define SD_PUNC		4423
#define SD_USER		4450

enum TONEe
{
	TONEBAD = -1, TONE1 = 1, TONE2, TONE3, TONE4, TONE5
};

//enum {CONSONANT_RESERVE, CONSONANT_ADJUST, CONSONANT_MERGE2VOWEL};
inline int HasTone(SNDID soundid)
{
//	return (0 < soundid && soundid < SD_ENG);
	return (0 < soundid && soundid < SD_SYSTEM + 5);
}

inline int Tone(SNDID soundid)
{
	if(!HasTone(soundid))
		return TONEBAD;
	if(soundid < SD_DIGIT)
		return soundid % 10;
	if(soundid < SD_PHONEME)
		return 1; // TMP toneOfDigit[soundid-SD_DIGIT];
	if(soundid < SD_ENG)
		return 1; // tone of all phoneme
	return TONEBAD;
}

inline int SetTone(SNDID &soundid, int tone)
{
	if(HasTone(soundid))
		soundid = (SNDID) (soundid - Tone(soundid) + tone);
	return soundid;
}

class CString;

SNDID Phone2SID(UCHAR *phonestr);
short GetBig5Index(UCHAR *b5);
//char *SID2Phone(SNDID soundid);
void SID2Phone(SNDID soundid, char* s);
//int Big52Phone(UCHAR *str, char maxnum, char phone[6][9]);
int Big52Phone(UCHAR *str, char phone[6][9]);
int Big52SID(UCHAR *str, SNDID *sid);
void GenerateSidPhoneBig5Table();
void CV_Group(SNDID soundid, char &c_group, char &v_group, char &c_id, char &v_id);
BOOL VoicedType(SNDID soundid);
void SID2InitialFinal(SNDID soundid, char *initial, char *final);
void pinyin2Phone(CString &pinyin, char *s, int &tone_p);

//=========================================================
//	chinese string tools
//=========================================================

inline WORD GetBig5Code(char *s)
{
	return WORD(WORD((BYTE) s[0] << 8) + (BYTE) s[1]);
}
inline int IsBig5(char *s)
{
	return (((BYTE) s[0] >= 0x80 && (BYTE) s[1] >= 0x40) ? TRUE : FALSE);
}

extern char bigPHONEME[];
extern SNDID *ttsPHONETAB;
//extern char *puncSENBIG5, *puncSENENG, *puncBIG5, *puncENG;

int PhonemeType(char ch, char &type);
int IsPunc(char *s);

