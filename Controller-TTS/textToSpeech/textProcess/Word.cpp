#include "Word.h"
#include "Phone.h"
#include "CString.h"
#include "LogHandler.h"
#include <cmath>
#include "CConvert.h"

#define WORD_FILE "WORD.DAT"
#define WORD_INDEX_FILE "WORD.NDX"
#define PHONE_FILE "PHONE.DAT"

using namespace std;

int wordPitchNdx[4] = { 0, 5, 55, 430 }; //5, 5*5*2=50, 5*5*5*3=375

static USHORT word_index_boundry = 0;
static unsigned short *word_index = NULL;
static WORD_DB *word_data = 0;

static unsigned char symbol[96][4] = { "　", "！", "”", "＃", "＄", "％", "＆", "’", "（", "）", "＊", "＋", "，", "－", "．", "／",
		"０", "１", "２", "３", "４", "５", "６", "７", "８", "９", "：", "；", "＜", "＝", "＞", "？", "＠", "Ａ", "Ｂ", "Ｃ", "Ｄ", "Ｅ",
		"Ｆ", "Ｇ", "Ｈ", "Ｉ", "Ｊ", "Ｋ", "Ｌ", "Ｍ", "Ｎ", "Ｏ", "Ｐ", "Ｑ", "Ｒ", "Ｓ", "Ｔ", "Ｕ", "Ｖ", "Ｗ", "Ｘ", "Ｙ", "Ｚ", "〔",
		"＼", "〕", "︿", "ˍ", "’", "Ａ", "Ｂ", "Ｃ", "Ｄ", "Ｅ", "Ｆ", "Ｇ", "Ｈ", "Ｉ", "Ｊ", "Ｋ", "Ｌ", "Ｍ", "Ｎ", "Ｏ", "Ｐ", "Ｑ",
		"Ｒ", "Ｓ", "Ｔ", "Ｕ", "Ｖ", "Ｗ", "Ｘ", "Ｙ", "Ｚ", "｛", "｜", "｝", "～", "～" };

unsigned char numberic[][4] = { "零", "一", "二", "三", "四", "五", "六", "七", "八", "九", "十", "百", "千", "萬", "億", "兆", "半",
		"幾", "多", "少", "壹", "貳", "參", "肆", "伍", "陸", "柒", "捌", "玖", "拾", "佰", "仟", "廿" };
static int num = 33;

static unsigned char tail_symbol[20] = "，。！？︰；";

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
			if (word_index[i] == 0xffff)
				continue;
			if (word_index[i] < j)
				break;
			j = word_index[i];
		}
		word_index_boundry = i;
	}

	// 啟峻加入修改的部分
//	m_word_data = word_data;
//	m_word_index = word_index;
	_log("[CWord] InitWord success");
}

void CWord::GetWord(WORD_PACKAGE &wordPackage)
{
	int i, j, k, start, ptr, offset;
	long ndx;
	UCHAR *leading;
	WORD_DB *wdb;

	// text_len是以Unicode計算
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

	// 設定int char_type[SENTENCE_LEN+1]的內容
	// char_type的內容可以為CHINESE_CHAR, ENGLISH_CHAR, DIGIT_CHAR等等，如下所列
	// enum {CHINESE_CHAR, ENGLISH_CHAR, DIGIT_CHAR, SYMBOL_CHAR, DOT_CHAR, SPECIAL_DIGIT, SPECIAL_CHAR, MOUSE_CHAR};
	SetCharType(wordPackage);

	// ptr是此函式中宣告的區域變數，型態為int
	// ptr在每次迴圈中都加2，是因為Unicode編碼的關係
	// 而text_len要成二的原因也是
	for (ptr = 0; ptr < wordPackage.txt_len * 2; ptr += 2)
	{
		// found也是區域變數，型態為BOOL
//		found = FALSE;
		// 如果輸入字串的內容中有"．"，就將他換成"點"，並更改對應之char_type[]

		if (memcmp(&wordPackage.txt[ptr], "．", 2) == 0)
		{
			memcpy(&wordPackage.txt[ptr], "點", 2);
			wordPackage.char_type[ptr / 2] = CHINESE_CHAR;
		}
		if (wordPackage.txt[ptr] > 0xF9 || wordPackage.txt[ptr] < 0xA4)
			continue;
		// ndx為區域變數，作為word的index，型態為長整數long
		ndx = (*(wordPackage.txt + ptr) - 0xa4) * 157;
		ndx += (*(wordPackage.txt + ptr + 1) > 0xa0) ?
				*(wordPackage.txt + ptr + 1) - 0xa1 + 0x3f : *(wordPackage.txt + ptr + 1) - 0x40;

		// leading為區域變數型態為UCHAR*也就是unsigned char*
		leading = NULL;
		if (word_index[ndx] != 0xffff)
		{
			offset = word_index[ndx];
			if (ndx >= word_index_boundry)
				offset += 65535L;
			for (i = offset;; i++)
			{
				// 由詞庫中讀出以輸入字串中目前index到的字為首的所有的詞
				// word_data為詞庫，指標為ndx
				int dsize = sizeof(*word_data);
				int psize = sizeof(word_data);
				int ssize = sizeof(word_data[i]);
				//std::cout << dsize << psize << ssize;
				wdb = &word_data[i];
				if (leading != NULL && (leading[0] != wdb->big5[0] || leading[1] != wdb->big5[1]))
					break;
				leading = wdb->big5;
				j = wdb->byte;

				// 只處理到詞長最大為四字詞
				if (j > 2 * 4)
					continue;

				// 根據詞長j，比較是否相同，回傳值若為零，表示一模一樣
				if (memcmp(wordPackage.txt + ptr, wdb->big5, j) == 0)
				{
					start = ptr / 2;
					int k;
					if (j == 2)
					{
						k = 1;
						wordPackage.tab[start][1] = start;
					}
					else
					{
						k = ++wordPackage.tab[start][0];
					}
					wordPackage.tab[start][k] = start + j / 2 - 1;
					wordPackage.ptrtab[start][k] = i;
				}
			}
		}
	}
	short *p;
	WORD_INFO *pwi;
	WORD_DB *pwdb;

	start = ndx = 0;
	wordPackage.best_score = -9999;
	Score(0, wordPackage);

	while (1)
	{
		WORD_INFO word_info;
		wordPackage.vecWordInfo.push_back(word_info);
		pwi = &wordPackage.vecWordInfo[wordPackage.wnum];

		wordPackage.vecWordInfo[wordPackage.wnum].sen_pos = 1; //先假設詞在句子的中間

		if (wordPackage.ptrtab[start][wordPackage.best[start]] >= 0)
		{
			pwdb = &word_data[wordPackage.ptrtab[start][wordPackage.best[start]]];
			memcpy(pwi->attr, pwdb->attr, 4);
		}
		else
			memset(pwi->attr, 0, 4);

		if (wordPackage.tab[start][wordPackage.best[start]] == start)
		{
			pwi->phone[0] = GetPhone(start, wordPackage);
			memcpy(pwi->big5, &wordPackage.txt[ndx * 2], 2);
			pwi->wlen = 1;
			ndx++;
		}
		else
		{
			p = pwdb->phone;
			j = wordPackage.tab[start][wordPackage.best[start]] - start + 1;
			memcpy(pwi->big5, pwdb->big5, j * 2);
			pwi->wlen = j;
			ndx += j;
			for (i = 0; i < j; i++)
			{
				pwi->phone[i] = p[i];
			}
		}
		wordPackage.wnum++;
		start = wordPackage.tab[start][wordPackage.best[start]] + 1;
		if (start >= wordPackage.txt_len)
			break;
	}

	if (wordPackage.char_type[wordPackage.txt_len / 2]
			== CHINESE_CHAR&& wordPackage.vecWordInfo[wordPackage.wnum - 1].phone[0] >= SD_PUNC)
	{
		pwi->sen_pos = 2;
		if (wordPackage.wnum > 2)
		{
			wordPackage.vecWordInfo[0].sen_pos = 0;
			wordPackage.vecWordInfo[wordPackage.wnum - 2].sen_pos = 2;
		}
	}
	else if (wordPackage.wnum > 1)
	{
		wordPackage.vecWordInfo[0].sen_pos = 0;
		pwi->sen_pos = 2;
	}
	//ChangePhone(wordPackage);

	start = 0;
	int t, t1, t2, t3;
	t1 = t2 = t3 = 0;

	for (i = 0; i < wordPackage.wnum; i++)
	{
		pwi = &wordPackage.vecWordInfo[i];
		wordPackage.toneComb4[i] = wordPackage.toneComb[i] = 0;
		for (k = start, j = 0; j < pwi->wlen; j++, k++)
		{
			if (wordPackage.char_type[k] == CHINESE_CHAR && pwi->phone[j] < SD_PUNC)
			{
				t = Tone(pwi->phone[j]) - 1;
				wordPackage.toneComb[i] = wordPackage.toneComb[i] * 5 + t;
				if (t == 4)
					wordPackage.toneComb4[i] = -100000;
				else
					wordPackage.toneComb4[i] = wordPackage.toneComb4[i] * 4 + t;
			}
			else
			{
				wordPackage.toneComb4[i] = wordPackage.toneComb[i] = -100000;
				break;
			}
		}

		for (j = 0, k = start; j < pwi->wlen; j++, k++)
		{		//不能與上面的合併：因為有 break;
			t3 = Tone(pwi->phone[j]);
			if (t3 == TONEBAD || t3 < 1 || t3 > 5)
			{
				t1 = t2 = t3 = 0;
				wordPackage.voicedType[k] = 0;
			}
			else
			{
				t3--;
				wordPackage.voicedType[k] = VoicedType(pwi->phone[j]);
			}
			wordPackage.sentenceToneCobm[k] = t3 + t2 * 5 + t1 * 25;
			t1 = t2;
			t2 = t3;
		}
		start += pwi->wlen;
	}
}

void CWord::Score(int cur_ptr, WORD_PACKAGE &wordPackage)
{
	short count, start;
	short i;

	if (cur_ptr >= wordPackage.txt_len)
	{
		count = 9000, start = 0;
		while (1)
		{
			if (wordPackage.tab[start][wordPackage.q[start]] == start)
				count -= 20;
			else
				count -= 10;
			start = wordPackage.tab[start][wordPackage.q[start]] + 1;
			if (start >= wordPackage.txt_len)
				break;
		}
		if (count >= wordPackage.best_score)
		{
			wordPackage.best_score = count;
			for (i = 0; i < wordPackage.txt_len; i++)
			{
				wordPackage.best[i] = wordPackage.q[i];
			}
		}
		return;
	}
	for (i = 1; i <= wordPackage.tab[cur_ptr][0]; i++)
	{
		wordPackage.q[cur_ptr] = i;
		Score(wordPackage.tab[cur_ptr][i] + 1, wordPackage);
	}
}

void CWord::SetCharType(WORD_PACKAGE &wordPackage)
{
	UINT v;
	UCHAR *p;
	int i, j;

	for (i = 0; i < wordPackage.txt_len; i++)
	{
		p = &wordPackage.txt[i * 2];
		v = p[0] * 256 + p[1];
		if (v >= 0xa2af && v <= 0xa2b8)
		{ //０１２３４５６７８９
			wordPackage.char_type[i] = DIGIT_CHAR;
			//strncpy((char *) &wordPackage.txt[2 * i], (char*) &numberic[v - 0xa2af][0], 2);
		}
		else if (v >= 0xa2cf && v <= 0xa2fe)
		{ //ＡＢＣ．．．Ｚ
			wordPackage.char_type[i] = ENGLISH_CHAR;
		}
		else if (v == 0xa144)
		{  //  .
			wordPackage.char_type[i] = DOT_CHAR;
		}
		else if (v == 0xa249)
		{
			wordPackage.char_type[i] = MOUSE_CHAR;
		}
		else
			wordPackage.char_type[i] = CHINESE_CHAR;
	}

	for (i = 0; i < wordPackage.txt_len; i++)
	{
		if (wordPackage.char_type[i] == DOT_CHAR)
		{
			if (i && i < wordPackage.txt_len - 1
					&& (wordPackage.char_type[i - 1] == ENGLISH_CHAR || wordPackage.char_type[i - 1] == DIGIT_CHAR
							|| wordPackage.char_type[i - 1] == SPECIAL_DIGIT) &&
					//^^^^^^^^SPECIAL_DIGIT ??
					(wordPackage.char_type[i + 1] == ENGLISH_CHAR || wordPackage.char_type[i + 1] == DIGIT_CHAR))
			{
				strncpy((char*) &wordPackage.txt[2 * i], "點", 2);
				for (j = i - 1; j >= 0; j--)
				{
					if (wordPackage.char_type[j] == ENGLISH_CHAR || wordPackage.char_type[j] == DIGIT_CHAR)
					{
						if (wordPackage.char_type[j] == DIGIT_CHAR)
							wordPackage.char_type[j] = SPECIAL_DIGIT;
					}
					else
						break;
				}
				for (i++; i < wordPackage.txt_len; i++)
				{
					if (wordPackage.char_type[i] == ENGLISH_CHAR || wordPackage.char_type[i] == DIGIT_CHAR)
					{
						if (wordPackage.char_type[i] == DIGIT_CHAR)
							wordPackage.char_type[i] = SPECIAL_DIGIT;
					}
					else
					{
						i--;
						break;
					}
				}
			}
			else
				wordPackage.char_type[i] = SYMBOL_CHAR;
		}
	}
}

unsigned CWord::GetPhone(int ptr, WORD_PACKAGE &wordPackage)
/* return 0 if not a voice_able  word */
/* return 1 ~ 27965 if a chinese voice_able word */
/* return 30000 if a english word */
{
	UCHAR buf[22];
	//unsigned best=0,offset ;
	SNDID sid[10];

	buf[0] = wordPackage.txt[ptr * 2];
	buf[1] = wordPackage.txt[ptr * 2 + 1];
	buf[2] = 0;
	if (buf[0] < 0xa4 || buf[0] > 0xf9)
		return SD_PUNC;
	// buf是斷出來的一個個的詞
	// 例如"我為人人，人人為我"中的"我"
	Big52SID(buf, sid);
	return sid[0];
}

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
		if (ch[0] == 0x1A)
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

			if (ch[0] == 0xa1 && ch[1] == 0x40)
				continue;

			p = strstr((char*) tail_symbol, (char*) ch);
			if ((p != NULL) && (((char*) p - (char*) tail_symbol) % 2 == 0))
			{
				len++;
				goto ret;
			}

			if (len > WORD_LEN2)
				if (wordPackage.txt[len * 2] == 0xA1)
					if ((wordPackage.txt[len * 2 + 1] >= 0x40) && (wordPackage.txt[len * 2 + 1] <= 0x7F)) /* symbol */
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
		//_log("[CWord] GetSentence ch[1]: %hhx", ch[1]);

		if (memcmp(ch, "卅", 2) == 0)
		{
			memcpy(&wordPackage.txt[len * 2], "三十", 4);
			len++;
		}
		else if (memcmp(ch, "廿", 2) == 0)
		{
			memcpy(&wordPackage.txt[len * 2], "二十", 4);
			len++;
		}
		else
		{
			memcpy(&wordPackage.txt[len * 2], ch, 2);
		}
		ch[2] = 0;
		if (len && ch[0] == 0xa1 && ch[1] == 0x40 && wordPackage.txt[len * 2 - 2] == 0xa1
				&& wordPackage.txt[len * 2 - 1] == 0x40)
			continue;
		p = strstr((char*) tail_symbol, (char*) ch);
		if (p != NULL)
		{
			if (memcmp(p, "．", 2) == 0)
			{
				memcpy(&wordPackage.txt[len * 2], "點", 2);
			}
			else if (((char*) p - (char*) tail_symbol) % 2 == 0)
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

BOOL CCsame(UCHAR * s, char* esi); //one Chinese Character Comparison
BOOL CCsame(UCHAR * s, char* esi)
{

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
	return 0;
	//same: return 1;
}

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

void CWord::ChangePhone(WORD_PACKAGE &wordPackage)
{
	int /*i,*/n, tone, prev_tone;
	WORD_INFO *pwi/*, *next_pwi*/;
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

