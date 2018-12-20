#include "Word.h"
#include "Phone.h"
#include "CString.h"
#include "LogHandler.h"
#include <cmath>
#include "CConvert.h"
#include <fstream>
#include <stdio.h>
#include "utf8.h"

#define WORD_FILE "WORD.DAT"
#define WORD_INDEX_FILE "WORD.NDX"
#define PHONE_FILE "PHONE.DAT"

using namespace std;

static USHORT word_index_boundry = 0;
static unsigned short *word_index = NULL;
static WORD_DB *word_data = 0;

unsigned char numberic[][4] = { "零", "一", "二", "三", "四", "五", "六", "七", "八", "九", "十", "百", "千", "萬", "億", "兆", "半",
		"幾", "多", "少", "壹", "貳", "參", "肆", "伍", "陸", "柒", "捌", "玖", "拾", "佰", "仟", "廿" };
static int num = 33;

CWord::CWord()
{

}

CWord::~CWord()
{

}

void CWord::InitWord(LPCTSTR dir)
{
	FILE* f;
	CString cs;
	int len;

	//==================== 載入字詞字典檔 =======================//
	ifstream file("model/WordData.txt");
	string str;
	if (file.is_open())
	{
		char * pch;
		int nIndex;

		while (getline(file, str))
		{
			vector<WORD_DIC> vecWord;
			WORD_DIC worddic;
			pch = strtok(const_cast<char*>(str.c_str()), ","); // 抓字
			if (NULL != pch)
			{
				worddic.strWord = pch;
				if (mapWordDictionary.end() == mapWordDictionary.find(utf8_substr(worddic.strWord, 0, 1)))
				{
					mapWordDictionary[utf8_substr(worddic.strWord, 0, 1)] = vecWord;
				}

				pch = strtok( NULL, ",");
				nIndex = 0;
				while (pch != NULL)
				{
					worddic.phoneID[nIndex] = atoi(pch);
					pch = strtok( NULL, ",");
					++nIndex;
				}

				mapWordDictionary[utf8_substr(worddic.strWord, 0, 1)].push_back(worddic);
			}

		}
		file.close();

#ifdef DEBUG
		for (map<std::string, vector<WORD_DIC> >::iterator it = mapWordDictionary.begin();
				it != mapWordDictionary.end(); ++it)
		{
			_log("<==================== %s =====================>", it->first.c_str());
			vector<WORD_DIC> vecDic = it->second;
			for (vector<WORD_DIC>::iterator vecit = it->second.begin(); it->second.end() != vecit; ++vecit)
			{
				_log("%s,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", vecit->strWord.c_str(), vecit->phoneID[0], vecit->phoneID[1],
						vecit->phoneID[2], vecit->phoneID[3], vecit->phoneID[4], vecit->phoneID[5], vecit->phoneID[6],
						vecit->phoneID[7], vecit->phoneID[8], vecit->phoneID[9]);
			}
		}
#endif
	}

//	mapWord
	/**********************FILE* method**********************************/

	if (0 == word_data)
	{
		cs.empty();
		cs = cs + dir + WORD_FILE;
		f = fopen(cs, "rb");
		fseek(f, 0, SEEK_END); // seek to end of file
		len = ftell(f); // get current file pointer
		word_data = new WORD_DB[(int) (len / sizeof(WORD_DB)) + 1];
		rewind(f); // seek back to beginning of file
		fread(word_data, sizeof(WORD_DB), floor(len / sizeof(WORD_DB)), f);
		fclose(f);
	}

	/*******************************************************************/

	if (0 == ttsPHONETAB)
	{
		cs.empty();
		cs = cs + dir + PHONE_FILE;
		f = fopen(cs, "rb");
		fseek(f, 0, SEEK_END); // seek to end of file
		len = ftell(f); // get current file pointer
		ttsPHONETAB = new short[(int) (len / sizeof(short)) + 1];
		rewind(f); // seek back to beginning of file
		fread(ttsPHONETAB, sizeof(short), floor(len / sizeof(short)), f);
		fclose(f);
	}

	if (0 == word_index)
	{
		cs.empty();
		cs = cs + dir + WORD_INDEX_FILE;
		f = fopen(cs, "rb");
		fseek(f, 0, SEEK_END); // seek to end of file
		len = ftell(f); // get current file pointer
		word_index = new unsigned short[(int) (len / sizeof(unsigned short)) + 1];
		rewind(f); // seek back to beginning of file
		fread(word_index, sizeof(unsigned short), floor(len / sizeof(unsigned short)), f);
		fclose(f);

		int i;
		int j = 0;
		for (i = 1; i < CHINESE_INDEX_NUM; i++)
		{
			if (word_index[i] == 0xffff) //65535
				continue;
			if (word_index[i] < j)
				break;
			j = word_index[i];
		}
		word_index_boundry = i;
	}

	_log("[CWord] InitWord success");
}

void CWord::GetWord(WORD_PACKAGE &wordPackage)
{
	int i, j, k, start, offset;
	string strWord;
	string strWordRight;
	string strText;

	//================= UTF-8 Version Start ============================//
	int nTxtLen = wordPackage.strText.length();
	int nWordNum = utf8len(wordPackage.strText.c_str());
	_log("[CWord] GetWord txt byte = %d word number = %d", nTxtLen, nWordNum);

	int nDicWordNum;
	int nDicWord;
	int nWordLen;
	int nWnum = 0;
	int offseted = 0;
	map<std::string, vector<WORD_DIC> >::iterator itWordDic;
	vector<WORD_DIC>::iterator itVecDic;
	vector<WORD_DIC> vecDic;
	i = 0;
	strText = wordPackage.strText;

	while (1)
	{
		if (0 >= GetUtf8Word(strText.c_str(), nWnum + 1, offset))
			break;

		nWordLen = offset - offseted;
		strWord = strText.substr(i, nWordLen);
		strWordRight = strText.substr(i);
		offseted = offset;

		//=========== 查字典檔 ===============//
		vecDic = mapWordDictionary[strWord];
	//	_log("============ strWord: %s ====================", strWord.c_str());

		for (itVecDic = vecDic.begin(); vecDic.end() != itVecDic; ++itVecDic)
		{
		//	_log("======= word:%s   dic word: %s phone: %d", strWordRight.c_str(), itVecDic->strWord.c_str(),					itVecDic->phoneID[0]);
			nDicWordNum = nDicWord = 0;
			if (0 == strWordRight.compare(0, itVecDic->strWord.length(), itVecDic->strWord.c_str()))
			{
				_log("===== word: %s   compare ok %s =======================", strWordRight.c_str(),
						itVecDic->strWord.c_str());
				strWordRight = strWordRight.substr(0, itVecDic->strWord.length());
				nDicWord = itVecDic->strWord.length();
				nDicWordNum = utf8len(itVecDic->strWord.c_str());
			//	_log("======== word right: %s  dicword size: %d  dicwordNum: %d ===========", strWordRight.c_str(),						nDicWord, nDicWordNum);
				break;
			}
		}

		if (0 != nDicWord)
		{
			i = offset + nDicWord - nWordLen;
			nWnum += nDicWordNum;
			offseted += (nDicWord - nWordLen);
		}
		else
		{
			i = offset;
			++nWnum;
		}
		nDicWord = 0;

		if (nWnum > nWordNum)
			break;
	}
//================= UTF-8 Version End =============================//

	long ndx = 0;
	UCHAR *leading;
	WORD_DB *wdb;
	CConvert convert;
	char *txt;

	if (-1 != convert.UTF8toBig5((char*) wordPackage.strText.c_str(), &txt))
	{
		// text_len是以Unicode計算 幾個字
		for (i = 0; i < wordPackage.txt_len + 1; ++i)
		{
			// SENTENCE_LEN定義為100
			// 而tab的宣告方式為 int tab[SENTENCE_LEN][11]
			wordPackage.tab[i][0] = 1;
			wordPackage.tab[i][1] = i;
			// 以下ptrtab的宣告方式為int ptrtab[SENTENCE_LEN][11]
			wordPackage.ptrtab[i][1] = -1;
		}
		// tab會長成如下所示
		// 1 0 0 0 0 0 0 0 0 0 0
		// 1 1 0 0 0 0 0 0 0 0 0
		// 1 2 0 0 0 0 0 0 0 0 0
		// 1 3 0 0 0 0 0 0 0 0 0
		// ...
		// 共有text_len個row
		// ptrtab會長成如下所示
		// - -1 0 0 0 0 0 0 0 0 0
		// - -1 0 0 0 0 0 0 0 0 0
		// - -1 0 0 0 0 0 0 0 0 0
		// - -1 0 0 0 0 0 0 0 0 0
		// ...
		// 共有text_len個row

		// 以下回圈是處理在輸入的句子之字數以外，到SENTENCE_LEN之間tab[][]的內容
		for (; i < SENTENCE_LEN; ++i)
			wordPackage.tab[i][0] = 0;

		short *p;
		WORD_DB *pwdb;

		wordPackage.wnum = start = ndx = 0;

		while (1)
		{
			WORD_INFO word_info;
			wordPackage.vecWordInfo.push_back(word_info);

			//================== 分詞 ====================//

//		wordPackage.vecWordInfo[wordPackage.wnum].sen_pos = 1; //先假設詞在句子的中間

			if (wordPackage.ptrtab[start][1] >= 0)
			{
				pwdb = &word_data[wordPackage.ptrtab[start][1]];
				//		memcpy(wordPackage.vecWordInfo[wordPackage.wnum].attr, pwdb->attr, 4);
			}
//		else
//			memset(wordPackage.vecWordInfo[wordPackage.wnum].attr, 0, 4);

			if (wordPackage.tab[start][1] == start) // 抓一個字
			{
				wordPackage.vecWordInfo[wordPackage.wnum].phone[0] = GetPhone(start, txt);
				memcpy(wordPackage.vecWordInfo[wordPackage.wnum].big5, &txt[ndx * 2], 2);
				wordPackage.vecWordInfo[wordPackage.wnum].wlen = 1;
				++ndx;
				_log("[CWord] GetWord start=%d phone=%d big5=%hhx wnum=%d ===========================", start,
						wordPackage.vecWordInfo[wordPackage.wnum].phone[0],
						wordPackage.vecWordInfo[wordPackage.wnum].big5, wordPackage.wnum);
			}
			else
			{
				p = pwdb->phone;
				j = wordPackage.tab[start][1] - start + 1;
				memcpy(wordPackage.vecWordInfo[wordPackage.wnum].big5, pwdb->big5, j * 2);
				wordPackage.vecWordInfo[wordPackage.wnum].wlen = j;
				ndx += j;
				for (i = 0; i < j; i++)
				{
					wordPackage.vecWordInfo[wordPackage.wnum].phone[i] = p[i];
				}
				_log("[CWord] GetWord start=%d phone=%d big5=%hhx wnum=%d xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx", start,
						wordPackage.vecWordInfo[wordPackage.wnum].phone[0],
						wordPackage.vecWordInfo[wordPackage.wnum].big5, wordPackage.wnum);
			}

			char *utf8 = 0;
			char **pUtf8 = &utf8;
			if (-1 != convert.Big5toUTF8((char*) wordPackage.vecWordInfo[wordPackage.wnum].big5, pUtf8))
			{
				wordPackage.vecWordInfo[wordPackage.wnum].strSentence = utf8;
				free(utf8);
			}

			++wordPackage.wnum;
			start = wordPackage.tab[start][1] + 1;
			if (start >= wordPackage.txt_len)
				break;
		}

		if (wordPackage.vecWordInfo[wordPackage.wnum - 1].phone[0] >= SD_PUNC)
		{
//		wordPackage.vecWordInfo[wordPackage.wnum].sen_pos = 2;
			if (wordPackage.wnum > 2)
			{
//			wordPackage.vecWordInfo[0].sen_pos = 0;
//			wordPackage.vecWordInfo[wordPackage.wnum - 2].sen_pos = 2;
			}
		}
		else if (wordPackage.wnum > 1)
		{
//		wordPackage.vecWordInfo[0].sen_pos = 0;
//		wordPackage.vecWordInfo[wordPackage.wnum].sen_pos = 2;
		}

		start = 0;
		int t1, t2, t3;
		t1 = t2 = t3 = 0;

		for (i = 0; i < wordPackage.wnum; ++i)
		{
//		pwi = &wordPackage.vecWordInfo[i];
//		wordPackage.toneComb4[i] = wordPackage.toneComb[i] = 0;
			for (k = start, j = 0; j < wordPackage.vecWordInfo[i].wlen; j++, k++)
			{
				if (wordPackage.vecWordInfo[i].phone[j] < SD_PUNC)
				{
					//t = Tone(wordPackage.vecWordInfo[i].phone[j]) - 1;
//				wordPackage.toneComb[i] = wordPackage.toneComb[i] * 5 + t;
//				if (t == 4)
//					wordPackage.toneComb4[i] = -100000;
//				else
//					wordPackage.toneComb4[i] = wordPackage.toneComb4[i] * 4 + t;
				}
				else
				{
//				wordPackage.toneComb4[i] = wordPackage.toneComb[i] = -100000;
					break;
				}
			}

			for (j = 0, k = start; j < wordPackage.vecWordInfo[i].wlen; j++, k++)
			{		//不能與上面的合併：因為有 break;
				t3 = Tone(wordPackage.vecWordInfo[i].phone[j]);
				if (t3 == TONEBAD || t3 < 1 || t3 > 5)
				{
					t1 = t2 = t3 = 0;
					//			wordPackage.voicedType[k] = 0;
				}
				else
				{
					t3--;
					//			wordPackage.voicedType[k] = VoicedType(wordPackage.vecWordInfo[i].phone[j]);
				}
				wordPackage.sentenceToneCobm[k] = t3 + t2 * 5 + t1 * 25;
				t1 = t2;
				t2 = t3;
			}
			start += wordPackage.vecWordInfo[i].wlen;
		}
		free(txt);
	}
}

unsigned CWord::GetPhone(int ptr, char *txt)
/* return 0 if not a voice_able  word */
/* return 1 ~ 27965 if a chinese voice_able word */
/* return 30000 if a english word */
{
	UCHAR buf[22];
//unsigned best=0,offset ;
	SNDID sid[10];

	buf[0] = txt[ptr * 2];
	buf[1] = txt[ptr * 2 + 1];
	buf[2] = 0;
	if (buf[0] < 0xa4 || buf[0] > 0xf9)
		return SD_PUNC;
// buf是斷出來的一個個的詞
// 例如"我為人人，人人為我"中的"我"
	Big52SID(buf, sid);
	return sid[0];
}

/*
 int CWord::GetSentence(UCHAR * from, int *textNdx, WORD_PACKAGE &wordPackage)
 {
 unsigned char ch[3];
 int len = 0;
 char *p;

 ch[2] = 0;
 _log("[CWord] GetSentence text: %s", from);
 while ((ch[0] = from[(*textNdx)++]) != 0)
 {
 //_log("[CWord] GetSentence ch[0]: %hhx", ch[0]);
 if (ch[0] == 0x1A) // 0X1A经过读取之后被处理成0XFF（即EOF（-1））
 {
 if (len == 0)
 len = -1;
 goto ret;
 }
 if (ch[0] == 0x0D)
 {
 if ((ch[1] = from[(*textNdx)++]) != 0x0A)
 _log("error: 0x0d not followed 0x0a");
 continue;
 }
 if (ch[0] < 128) // English letter
 {
 if (ch[0] < 0x20)
 ch[0] = 0x20;

 memcpy(&wordPackage.txt[len * 2], symbol[ch[0] - 0x20], 2);
 memcpy(ch, &wordPackage.txt[len * 2], 2);
 ch[2] = 0;

 if (ch[0] == 0xa1 && ch[1] == 0x40) // 0xa1，说明输入的是汉字。因为汉字的内码是从0xa1开始编码的
 continue;

 p = strstr((char*) tail_symbol, (char*) ch);
 if ((p != NULL) && (((char*) p - (char*) tail_symbol) % 2 == 0))
 {
 len++;
 goto ret;
 }

 if (len > WORD_LEN2)
 if (wordPackage.txt[len * 2] == 0xA1)
 if ((wordPackage.txt[len * 2 + 1] >= 0x40) && (wordPackage.txt[len * 2 + 1] <= 0x7F))
 {
 len++;
 goto ret;
 }

 len++;
 if (len > WORD_LEN2)
 goto ret;
 continue;
 }

 ch[1] = from[(*textNdx)++];

 memcpy(&wordPackage.txt[len * 2], ch, 2);

 ch[2] = 0;
 if (len && ch[0] == 0xa1 && ch[1] == 0x40 && wordPackage.txt[len * 2 - 2] == 0xa1
 && wordPackage.txt[len * 2 - 1] == 0x40)
 continue;
 p = strstr((char*) tail_symbol, (char*) ch);
 if (p != NULL)
 {
 if (((char*) p - (char*) tail_symbol) % 2 == 0)
 {
 len++;
 goto ret;
 }
 }

 if (len > WORD_LEN2 * 2)
 if (wordPackage.txt[len * 2] == 0xA1)
 if ((wordPackage.txt[len * 2 + 1] > 0x40) && (wordPackage.txt[len * 2 + 1] < 0x7F)) // symbol
 {
 len++;
 goto ret;
 }
 len++;
 if (len > WORD_LEN2 * 2)
 goto ret;
 }

 (*textNdx)--;
 _log("[CWord] GetSentence txt_len: %d", len);
 ret: wordPackage.txt_len = len;

 if (len > 0)
 {
 wordPackage.txt[len * 2] = 0;
 return len;
 }
 return (-1);
 }
 */
int CWord::IsNumberic(unsigned char *ch)
{
	int i;

	for (i = 0; i < num; i++)
	{
		if (memcmp(numberic[i], ch, 2) == 0)
			break;
	}
	if (i != num)
		return (1);
	else
		return (0);
}

void CWord::SetTone(int wno, int ndx, USHORT new_tone, WORD_PACKAGE &wordPackage)
{
	wordPackage.vecWordInfo[wno].phone[ndx] = wordPackage.vecWordInfo[wno].phone[ndx] / 10 * 10 + new_tone;
}

//BOOL CCsame(UCHAR * s, char* esi); //one Chinese Character Comparison
//BOOL CCsame(UCHAR * s, char* esi)
//{

/*
 asm
 (
 "mov eax,dword ptr [esi]"
 "mov eax,[eax]"
 "and eax,0xffff"
 "xchg ah,al"
 "cmp eax,dword ptr [esi]"
 "je same"
 );*/
//return 0;
//same: return 1;
//}
const char *Pohin[][2] = { { "石", "ㄉㄢˋ" }, { "任", "ㄖㄣˋ" }, { "曲", "ㄑㄩˇ" }, { "行", "ㄏㄤˊ" }, { "更", "ㄍㄥ" },
		{ "度", "ㄉㄨˋ" }, { "省", "ㄕㄥˇ" }, { "重", "ㄔㄨㄥˊ" }, { "校", "ㄒㄧㄠˋ" }, { "處", "ㄔㄨˋ" }, { "著", "ㄓㄠ" }, { "載", "ㄗㄞˇ" },
		{ "種", "ㄓㄨㄥˇ" }, { "擔", "ㄉㄢˋ" }, { "子", "ㄗˇ" }, { "角", "ㄐㄧㄠˇ" }, { "卷", "ㄐㄩㄢˇ" },
		{ "刻", "ㄎㄜˋ" }, //num
		{ "宿", "ㄒㄧㄡˇ" }, //num
		{ "朝", "ㄓㄠ" }, //num
		{ "間", "ㄐㄧㄢ" }, { "剎", "ㄔㄚˋ" }, { "吐", "ㄊㄨˇ" }, { "畜", "ㄔㄨˋ" }, { "較", "ㄐㄧㄠˋ" }, { "說", "ㄕㄨㄛ" },
		{ "轉", "ㄓㄨㄢˇ" }, { "騎", "ㄐㄧˋ" }, { "鵠", "ㄏㄨˊ" }, { "覺", "ㄐㄧㄠˋ" }, { "勺", "ㄕㄠˊ" }, { "種", "ㄓㄨㄥˇ" },
		//{"分" , "ㄈㄣ"},
		//{"匹" , "ㄆㄧ"    , "ㄆㄧˇ"  },
		{ 0, 0 } };
/**/
/**/
/**************************************2007 09 19 fable*****/
/*假設資料庫中資料全部為正確的資料						   */
/*所以對斷出來的詞不再做sandhi rule的轉變				   */
/*只針對詞與詞之間做sandhi rule的轉換					   */
/**************************************2007 09 19 fable*****/
/*
 void CWord::ChangePhone(WORD_PACKAGE &wordPackage)
 {
 int n, tone, prev_tone;
 WORD_INFO *pwi;
 UCHAR *first_char;
 UCHAR tmp = 0;
 bool flag;   //for 一

 prev_tone = -1;
 flag = false;
 first_char = &tmp;
 for (n = wordPackage.wnum - 1; n >= 0; n--)
 {
 pwi = &wordPackage.vecWordInfo[n];
 //3+3 rule between phones
 tone = Tone(pwi->phone[pwi->wlen - 1]);
 if (prev_tone == 3 && tone == 3)
 {
 SetTone(n, pwi->wlen - 1, 2, wordPackage);
 }

 if (pwi->wlen == 1)
 {
 //數字+ 種 個
 if (IsNumberic(pwi->big5))
 {
 if (CCsame(first_char, "種"))
 {
 SetTone(n + 1, 0, 3, wordPackage);
 prev_tone = 3;
 }
 if (CCsame(first_char, "個"))
 {
 SetTone(n + 1, 0, 5, wordPackage);
 prev_tone = 5;
 }
 }
 //一's rule
 if (flag)   //數字 非 兆 億 萬 千 百+一
 {
 if (IsNumberic(pwi->big5))
 SetTone(n + 1, 0, 1, wordPackage);
 flag = false;
 }

 if (CCsame(pwi->big5, "一"))
 {
 //一在最後時
 if (n == wordPackage.wnum - 1)
 {
 SetTone(n, 0, 1, wordPackage);
 continue;
 }
 //+(4 || 5)=>ㄧˊ      ex 一次 一個
 //+(1 || 2 || 3)=>ㄧˋ ex 一支 一人 一把
 if (prev_tone == 4 || prev_tone == 5)
 SetTone(n, 0, 2, wordPackage);
 else
 SetTone(n, 0, 4, wordPackage);

 if (!IsNumberic(first_char))
 {
 flag = true;				//標記上一個字為一+非數字時的情況 用以數字+一+非數字時改調
 }
 else				//一後面是數字時  非 兆 億 萬 千 百
 {
 if (CCsame(first_char, "兆") || CCsame(first_char, "億") || CCsame(first_char, "萬")
 || CCsame(first_char, "千") || CCsame(first_char, "百"))
 continue;
 SetTone(n, 0, 1, wordPackage);
 flag = false;
 }
 }
 //不
 //+(4)=>ㄅㄨˊ
 //+(1 || 2 || 3)=> ㄅㄨˋ
 if (CCsame(pwi->big5, "不"))
 {
 if (prev_tone == 4 || prev_tone == 5)
 SetTone(n, 0, 2, wordPackage);
 else
 SetTone(n, 0, 4, wordPackage);
 }
 if (CCsame(pwi->big5, "教"))
 {
 SetTone(n, 0, 1, wordPackage);
 }
 if (CCsame(pwi->big5, "種"))
 {
 SetTone(n, 0, 4, wordPackage);
 }
 if (CCsame(first_char, "正"))
 {
 if (CCsame(pwi->big5, "圓") || CCsame(pwi->big5, "元"))
 {
 SetTone(n + 1, 0, 3, wordPackage);
 }
 }
 }

 first_char = &pwi->big5[0];
 prev_tone = Tone(pwi->phone[0]);
 }
 }
 */
