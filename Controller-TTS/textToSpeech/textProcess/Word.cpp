#include "Word.h"

int wordPitchNdx[4] = { 0, 5, 55, 430 }; //5, 5*5*2=50, 5*5*5*3=375

static unsigned short word_index_boundry = 0;
static unsigned short *word_index = NULL;
static WORD_DB *word_data;
static int init = 0;

#define WORD_FILE "word.dat"
#define WORD_INDEX_FILE "word.ndx"
#define PHONE_FILE "phone.dat"

static int num = 33;

static unsigned char tail_symbol[20] = "，。！？︰；"; //"．"

CWord::CWord() :
		m_word_data(0), m_word_index(0), txt_len(0), m_punctuation(0), wnum(1), best_score(0), m_init(false)
{

}

CWord::~CWord()
{
	if(m_init)
	{
		init--;
	}

}

void CWord::InitWord(const char* dir)
{

}

void CWord::GetWord()
{

}

void CWord::Score(int cur_ptr)
{
	short count, start;
	short i;

	if(cur_ptr >= txt_len)
	{
		count = 9000, start = 0;
		while(1)
		{
			if(tab[start][q[start]] == start)
				count -= 20;
			else
				count -= 10;
			start = tab[start][q[start]] + 1;
			if(start >= txt_len)
				break;
		}
		if(count >= best_score)
		{
			best_score = count;
			for(i = 0; i < txt_len; i++)
			{
				best[i] = q[i];
			}
		}
		return;
	}
	for(i = 1; i <= tab[cur_ptr][0]; i++)
	{
		q[cur_ptr] = i;
		Score(tab[cur_ptr][i] + 1);
	}
}

void CWord::SetCharType()
{

}

unsigned CWord::GetPhone(int ptr)
/* return 0 if not a voice_able  word */
/* return 1 ~ 27965 if a chinese voice_able word */
/* return 30000 if a english word */
{

	return 0;
}

int CWord::ReadText(std::ofstream* fp) /* return 0 if end of file */
{

	return (-1);
}

int CWord::GetSentence(unsigned char * from, int *textNdx) /*return 0 if end*/
{

	return (-1);
}

int CWord::IsNumberic(unsigned char *ch)
{
	return (0);
}

void CWord::SetTone(int wno, int ndx, unsigned short new_tone)
{
	w_info[wno].phone[ndx] = w_info[wno].phone[ndx] / 10 * 10 + new_tone;
}

bool CCsame(unsigned char * s, const int code); //one Chinese Character Comparison
bool CCsame(unsigned char * s, const int code)
{
	/*	int i=(s[0]<<8) + s[1];
	 return (i==code);
	 */

	return 0;

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

void CWord::ChangePhone()
{

}
/**************************************2007 09 19 fable*****/
