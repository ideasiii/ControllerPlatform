#pragma once
#include <vector>
#include <string>
#include <memory.h>
#include <map>
#include "container.h"
#include "CString.h"

//注意：WA_VA11 ~ WA_VP : 不要更動其順序
enum WORD_ATTR
{
	WA_NONE = 0,
	WA_NUM,
	WA_NAME,
	WA_VA11,
	WA_VA12,
	WA_VA13,
	WA_VA2,
	WA_VA3,
	WA_VA4,
	WA_VB11,
	WA_VB12,
	WA_VB2,
	WA_VC1,
	WA_VC2,
	WA_VC31,
	WA_VC32,
	WA_VC33,
	WA_VD1,
	WA_VD2,
	WA_VE11,
	WA_VE12,
	WA_VE2,
	WA_VF1,
	WA_VF2,
	WA_VG1,
	WA_VG2,
	WA_VH11,
	WA_VH12,
	WA_VH13,
	WA_VH14,
	WA_VH15,
	WA_VH16,
	WA_VH17,
	WA_VH21,
	WA_VH22,
	WA_VI1,
	WA_VI2,
	WA_VI3,
	WA_VJ1,
	WA_VJ2,
	WA_VJ3,
	WA_VK1,
	WA_VK2,
	WA_VL1,
	WA_VL2,
	WA_VL3,
	WA_VL4,
	WA_V_11,
	WA_V_12,
	WA_V_2,
	WA_VP,
	WA_VR,
	WA_N,
	WA_Naa,
	WA_Nab,
	WA_Nac,
	WA_Nad,
	WA_Naea,
	WA_Naeb,
	WA_Nba,
	WA_Nbc,
	WA_Nca,
	WA_Ncb,
	WA_Ncc,
	WA_Ncda,
	WA_Ncdb,
	WA_Nce,
	WA_Ndaaa,
	WA_Ndaab,
	WA_Ndaac,
	WA_Ndaad,
	WA_Ndaba,
	WA_Ndabb,
	WA_Ndabc,
	WA_Ndabd,
	WA_Ndabe,
	WA_Ndabf,
	WA_Nhaca,
	WA_Nhacb,
	WA_Ndbb,
	WA_Ndc,
	WA_Ndda,
	WA_Nddb,
	WA_Nddc,
	WA_Ne,
	WA_Nfa,
	WA_Nfb,
	WA_Nfc,
	WA_Nfd,
	WA_Nfe,
	WA_Nff,
	WA_Nfg,
	WA_Nfh,
	WA_Nfi,
	WA_Nfzz,
	WA_Ng,
	WA_Nhaa,
	WA_Nhab,
	WA_Nhac,
	WA_Nhb,
	WA_Nhc,
	WA_NP,
	WA_P1,
	WA_P2,
	WA_P3,
	WA_P4,
	WA_P5,
	WA_P6,
	WA_P7,
	WA_P8,
	WA_P9,
	WA_P10,
	WA_P11,
	WA_P12,
	WA_P13,
	WA_P14,
	WA_P15,
	WA_P16,
	WA_P17,
	WA_P18,
	WA_P19,
	WA_P20,
	WA_P21,
	WA_P22,
	WA_P23,
	WA_P24,
	WA_P25,
	WA_P26,
	WA_P27,
	WA_P28,
	WA_P29,
	WA_P30,
	WA_P31,
	WA_P32,
	WA_P33,
	WA_P34,
	WA_P35,
	WA_P36,
	WA_P37,
	WA_P38,
	WA_P39,
	WA_P40,
	WA_P41,
	WA_P42,
	WA_P43,
	WA_P44,
	WA_P45,
	WA_P46,
	WA_P47,
	WA_P48,
	WA_P49,
	WA_P50,
	WA_P51,
	WA_P52,
	WA_P53,
	WA_P54,
	WA_P55,
	WA_P56,
	WA_P57,
	WA_P58,
	WA_P59,
	WA_P60,
	WA_P61,
	WA_P62,
	WA_P63,
	WA_P64,
	WA_P65,
	WA_A,
	WA_b,
	WA_Caa,
	WA_Cab,
	WA_Cbaa,
	WA_Cbab,
	WA_Cbba,
	WA_Cbbb,
	WA_Cbca,
	WA_Cbcb,
	WA_De,
	WA_Dia,
	WA_I,
	WA_RD,
	WA_S,
	WA_Str,
	WA_T,
	WA_Ta,
	WA_Tb,
	WA_Tc,
	WA_Td,
	WA_Da,
	WA_Dbaa,
	WA_Dbab,
	WA_Dbb,
	WA_Dbc,
	WA_Dc,
	WA_Dd,
	WA_Dfa,
	WA_Dfb,
	WA_Dg,
	WA_Dh,
	WA_Di,
	WA_Dj,
	WA_Dk,
	WA_V_LY,
	WA_M,
	WA_STAR
};

#define WORD_ATTR_NUM (WA_STAR+1)
//最大的詞數
//#define WORD_LEN 10
#define WORD_LEN2 (WORD_LEN*2)
#define SENTENCE_LEN 100

#pragma pack(push,1)
struct WORD_DB  //總共占 48 bytes (仲)
{
	char byte;				//詞的BYTE數 (1 byte)
	unsigned char attr[4];          //(4 bytes)
	unsigned char big5[20];  //big5,一字二占2 byte (20 bytes)
	unsigned int counter;          //字數*2 (1 byte)
	short phone[10];	//發音代碼 (10 bytes)
};
#pragma pack(pop)

struct WORD_INFO                //記錄斷詞的結果
{
	char wlen;					//詞的長度（以中文字為單位)
//	unsigned char big5[20 + 1];	//+1，當作字串結尾。這是詞的內容(可輸出)
//	char sen_pos;				//詞在句中的位置
//	short phone[WORD_LEN];		//發音代碼，也就是sound ID，是將big5轉成sid
	std::string strPhone[10];				// 羅馬拼音
	std::string strSentence;		// UTF8
};

struct WORD_PACKAGE
{
	int txt_len;							// 句子的字數
	int wnum;							//	word num，也就是這個句子被斷成幾個詞
	//unsigned char txt[200];	// 這個類別中所要處理的句子
	//char *txt;
	std::string strText;
	std::vector<WORD_INFO> vecWordInfo;

	int tab[SENTENCE_LEN][11];
	int ptrtab[SENTENCE_LEN][11];
//	int best[SENTENCE_LEN];
//	int q[SENTENCE_LEN * 2];
//	int toneComb[SENTENCE_LEN * 2], toneComb4[SENTENCE_LEN * 2];	//toneComb與toneComb4的差別好像是，一個是五聲調，一個是四聲調
//	int voicedType[SENTENCE_LEN * 2];			//有聲或無聲的子音
	int sentenceToneCobm[SENTENCE_LEN * 2];
//	int best_score;
//	int char_type[SENTENCE_LEN + 1];

public:
	void clear()
	{
		txt_len = 0;
		wnum = 0;
		//txt = 0;
		//	memset(txt, 0, sizeof(txt));
		vecWordInfo.clear();
	}
};

//========================= 字詞字典檔==============================//
typedef struct _WORD_DIC
{
	std::string strWord;
	//short phoneID[10];
	std::string strPhone[10];
} WORD_DIC;
//static std::map<std::string, short[10]> mapWord;

static std::map<std::string, vector<WORD_DIC> > mapWordDictionary;

//========================= 字詞分割===============================//
static std::vector<std::string> vWordWrap = { "。", "。", "。", "？", "！", "；", "，", "!", ",", ".", ";", "?", "，", "！", "，",
		"\r", ":", "、" };
//========================= 字詞删除===============================//
static std::vector<std::string> vWordDel = { "\n", "\r", "\t", " ", "	", "(", ")", "[", "]", "{", "}", "'", "、", "\"",
		"@", "%", "^", "&", "*", "”", "＃", "＄", "％", "＆", "’", "（", "）", "＊", "＋", "，", "－", "／", "＜", "＝", "＞", "？",
		"＠", "Ａ", "Ｂ", "Ｃ", "Ｄ", "Ｅ", "Ｆ", "Ｇ", "Ｈ", "Ｉ", "Ｊ", "Ｋ", "Ｌ", "Ｍ", "Ｎ", "Ｏ", "Ｐ", "Ｑ", "Ｒ", "Ｓ", "Ｔ", "Ｕ",
		"Ｖ", "Ｗ", "Ｘ", "Ｙ", "Ｚ", "〔", "＼", "〕", "︿", "ˍ", "’", "Ａ", "Ｂ", "Ｃ", "Ｄ", "Ｅ", "Ｆ", "Ｇ", "Ｈ", "Ｉ", "Ｊ", "Ｋ",
		"Ｌ", "Ｍ", "Ｎ", "Ｏ", "Ｐ", "Ｑ", "Ｒ", "Ｓ", "Ｔ", "Ｕ", "Ｖ", "Ｗ", "Ｘ", "Ｙ", "Ｚ", "｛", "｜", "｝", "～", "～", "。", "。",
		"。", "！", "，", ":", "、", " " };
//========================= 字詞替換===============================//

static std::map<std::string, std::string> mapWordExchange1 =
		create_map<std::string, std::string>("10", "十")("20", "二十")("30", "三十")("40", "四十")("50", "五十")("60", "六十")(
				"70", "七十")("80", "八十")("90", "九十");
static std::map<std::string, std::string> mapWordExchange2 = create_map<std::string, std::string>("0", "零")("1", "一")(
		"2", "二")("3", "三")("4", "四")("5", "五")("6", "六")("7", "七")("8", "八")("9", "九");
static std::map<std::string, std::string> mapWordExchange3 = create_map<std::string, std::string>("卅", "三十")("廿", "二十")(
		"．", "點");
static std::vector<std::map<std::string, std::string> > vecMaps =
		{ mapWordExchange1, mapWordExchange2, mapWordExchange3 };

