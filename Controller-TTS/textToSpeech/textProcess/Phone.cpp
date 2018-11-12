#include "CString.h"
#include "Phone.h"
#include <map>

using namespace std;

const char *PHONE_TABLE[NUM_PHONE] = { "ㄓ", "ㄔ", "ㄕ", "ㄖ", "ㄗ", "ㄘ", "ㄙ", "ㄚ", "ㄓㄚ", "ㄔㄚ", "ㄕㄚ", "ㄗㄚ", "ㄘㄚ", "ㄙㄚ", "ㄍㄚ",
		"ㄎㄚ", "ㄏㄚ", "ㄉㄚ", "ㄊㄚ", "ㄋㄚ", "ㄌㄚ", "ㄅㄚ", "ㄆㄚ", "ㄇㄚ", "ㄈㄚ", "ㄛ", "ㄌㄛ", "ㄅㄛ", "ㄆㄛ", "ㄇㄛ", "ㄈㄛ", "ㄜ", "ㄓㄜ", "ㄔㄜ",
		"ㄕㄜ", "ㄖㄜ", "ㄗㄜ", "ㄘㄜ", "ㄙㄜ", "ㄍㄜ", "ㄎㄜ", "ㄏㄜ", "ㄉㄜ", "ㄊㄜ", "ㄋㄜ", "ㄌㄜ", "ㄞ", "ㄓㄞ", "ㄔㄞ", "ㄕㄞ", "ㄗㄞ", "ㄘㄞ", "ㄙㄞ",
		"ㄍㄞ", "ㄎㄞ", "ㄏㄞ", "ㄉㄞ", "ㄊㄞ", "ㄋㄞ", "ㄌㄞ", "ㄅㄞ", "ㄆㄞ", "ㄇㄞ", "ㄝ", "ㄓㄟ", "ㄕㄟ", "ㄗㄟ", "ㄙㄟ", "ㄍㄟ", "ㄏㄟ", "ㄉㄟ", "ㄋㄟ",
		"ㄌㄟ", "ㄅㄟ", "ㄆㄟ", "ㄇㄟ", "ㄈㄟ", "ㄠ", "ㄓㄠ", "ㄔㄠ", "ㄕㄠ", "ㄖㄠ", "ㄗㄠ", "ㄘㄠ", "ㄙㄠ", "ㄍㄠ", "ㄎㄠ", "ㄏㄠ", "ㄉㄠ", "ㄊㄠ", "ㄋㄠ",
		"ㄌㄠ", "ㄅㄠ", "ㄆㄠ", "ㄇㄠ", "ㄡ", "ㄓㄡ", "ㄔㄡ", "ㄕㄡ", "ㄖㄡ", "ㄗㄡ", "ㄘㄡ", "ㄙㄡ", "ㄍㄡ", "ㄎㄡ", "ㄏㄡ", "ㄉㄡ", "ㄊㄡ", "ㄋㄡ", "ㄌㄡ",
		"ㄆㄡ", "ㄇㄡ", "ㄈㄡ", "ㄢ", "ㄓㄢ", "ㄔㄢ", "ㄕㄢ", "ㄖㄢ", "ㄗㄢ", "ㄘㄢ", "ㄙㄢ", "ㄍㄢ", "ㄎㄢ", "ㄏㄢ", "ㄉㄢ", "ㄊㄢ", "ㄋㄢ", "ㄌㄢ", "ㄅㄢ",
		"ㄆㄢ", "ㄇㄢ", "ㄈㄢ", "ㄣ", "ㄓㄣ", "ㄔㄣ", "ㄕㄣ", "ㄖㄣ", "ㄗㄣ", "ㄘㄣ", "ㄙㄣ", "ㄍㄣ", "ㄎㄣ", "ㄏㄣ", "ㄋㄣ", "ㄅㄣ", "ㄆㄣ", "ㄇㄣ", "ㄈㄣ",
		"ㄤ", "ㄓㄤ", "ㄔㄤ", "ㄕㄤ", "ㄖㄤ", "ㄗㄤ", "ㄘㄤ", "ㄙㄤ", "ㄍㄤ", "ㄎㄤ", "ㄏㄤ", "ㄉㄤ", "ㄊㄤ", "ㄋㄤ", "ㄌㄤ", "ㄅㄤ", "ㄆㄤ", "ㄇㄤ", "ㄈㄤ",
		"ㄥ", "ㄓㄥ", "ㄔㄥ", "ㄕㄥ", "ㄖㄥ", "ㄗㄥ", "ㄘㄥ", "ㄙㄥ", "ㄍㄥ", "ㄎㄥ", "ㄏㄥ", "ㄉㄥ", "ㄊㄥ", "ㄋㄥ", "ㄌㄥ", "ㄅㄥ", "ㄆㄥ", "ㄇㄥ", "ㄈㄥ",
		"ㄧ", "ㄐㄧ", "ㄑㄧ", "ㄒㄧ", "ㄉㄧ", "ㄊㄧ", "ㄋㄧ", "ㄌㄧ", "ㄅㄧ", "ㄆㄧ", "ㄇㄧ", "ㄨ", "ㄓㄨ", "ㄔㄨ", "ㄕㄨ", "ㄖㄨ", "ㄗㄨ", "ㄘㄨ", "ㄙㄨ",
		"ㄍㄨ", "ㄎㄨ", "ㄏㄨ", "ㄉㄨ", "ㄊㄨ", "ㄋㄨ", "ㄌㄨ", "ㄅㄨ", "ㄆㄨ", "ㄇㄨ", "ㄈㄨ", "ㄩ", "ㄐㄩ", "ㄑㄩ", "ㄒㄩ", "ㄋㄩ", "ㄌㄩ", "ㄧㄚ",
		"ㄐㄧㄚ", "ㄑㄧㄚ", "ㄒㄧㄚ", "ㄌㄧㄚ", "ㄧㄝ", "ㄐㄧㄝ", "ㄑㄧㄝ", "ㄒㄧㄝ", "ㄉㄧㄝ", "ㄊㄧㄝ", "ㄋㄧㄝ", "ㄌㄧㄝ", "ㄅㄧㄝ", "ㄆㄧㄝ", "ㄇㄧㄝ", "ㄧㄞ",
		"ㄧㄠ", "ㄐㄧㄠ", "ㄑㄧㄠ", "ㄒㄧㄠ", "ㄉㄧㄠ", "ㄊㄧㄠ", "ㄋㄧㄠ", "ㄌㄧㄠ", "ㄅㄧㄠ", "ㄆㄧㄠ", "ㄇㄧㄠ", "ㄧㄡ", "ㄐㄧㄡ", "ㄑㄧㄡ", "ㄒㄧㄡ", "ㄉㄧㄡ",
		"ㄋㄧㄡ", "ㄌㄧㄡ", "ㄇㄧㄡ", "ㄧㄢ", "ㄐㄧㄢ", "ㄑㄧㄢ", "ㄒㄧㄢ", "ㄉㄧㄢ", "ㄊㄧㄢ", "ㄋㄧㄢ", "ㄌㄧㄢ", "ㄅㄧㄢ", "ㄆㄧㄢ", "ㄇㄧㄢ", "ㄧㄣ", "ㄐㄧㄣ",
		"ㄑㄧㄣ", "ㄒㄧㄣ", "ㄋㄧㄣ", "ㄌㄧㄣ", "ㄅㄧㄣ", "ㄆㄧㄣ", "ㄇㄧㄣ", "ㄧㄤ", "ㄐㄧㄤ", "ㄑㄧㄤ", "ㄒㄧㄤ", "ㄋㄧㄤ", "ㄌㄧㄤ", "ㄧㄥ", "ㄐㄧㄥ", "ㄑㄧㄥ",
		"ㄒㄧㄥ", "ㄉㄧㄥ", "ㄊㄧㄥ", "ㄋㄧㄥ", "ㄌㄧㄥ", "ㄅㄧㄥ", "ㄆㄧㄥ", "ㄇㄧㄥ", "ㄨㄚ", "ㄓㄨㄚ", "ㄔㄨㄚ", "ㄕㄨㄚ", "ㄍㄨㄚ", "ㄎㄨㄚ", "ㄏㄨㄚ", "ㄨㄛ",
		"ㄓㄨㄛ", "ㄔㄨㄛ", "ㄕㄨㄛ", "ㄖㄨㄛ", "ㄗㄨㄛ", "ㄘㄨㄛ", "ㄙㄨㄛ", "ㄍㄨㄛ", "ㄎㄨㄛ", "ㄏㄨㄛ", "ㄉㄨㄛ", "ㄊㄨㄛ", "ㄋㄨㄛ", "ㄌㄨㄛ", "ㄨㄞ", "ㄓㄨㄞ",
		"ㄔㄨㄞ", "ㄕㄨㄞ", "ㄍㄨㄞ", "ㄎㄨㄞ", "ㄏㄨㄞ", "ㄨㄟ", "ㄓㄨㄟ", "ㄔㄨㄟ", "ㄕㄨㄟ", "ㄖㄨㄟ", "ㄗㄨㄟ", "ㄘㄨㄟ", "ㄙㄨㄟ", "ㄍㄨㄟ", "ㄎㄨㄟ", "ㄏㄨㄟ",
		"ㄉㄨㄟ", "ㄊㄨㄟ", "ㄨㄢ", "ㄓㄨㄢ", "ㄔㄨㄢ", "ㄕㄨㄢ", "ㄖㄨㄢ", "ㄗㄨㄢ", "ㄘㄨㄢ", "ㄙㄨㄢ", "ㄍㄨㄢ", "ㄎㄨㄢ", "ㄏㄨㄢ", "ㄉㄨㄢ", "ㄊㄨㄢ", "ㄋㄨㄢ",
		"ㄌㄨㄢ", "ㄨㄣ", "ㄓㄨㄣ", "ㄔㄨㄣ", "ㄕㄨㄣ", "ㄖㄨㄣ", "ㄗㄨㄣ", "ㄘㄨㄣ", "ㄙㄨㄣ", "ㄍㄨㄣ", "ㄎㄨㄣ", "ㄏㄨㄣ", "ㄉㄨㄣ", "ㄊㄨㄣ", "ㄌㄨㄣ", "ㄨㄤ",
		"ㄓㄨㄤ", "ㄔㄨㄤ", "ㄕㄨㄤ", "ㄍㄨㄤ", "ㄎㄨㄤ", "ㄏㄨㄤ", "ㄨㄥ", "ㄓㄨㄥ", "ㄔㄨㄥ", "ㄖㄨㄥ", "ㄗㄨㄥ", "ㄘㄨㄥ", "ㄙㄨㄥ", "ㄍㄨㄥ", "ㄎㄨㄥ", "ㄏㄨㄥ",
		"ㄉㄨㄥ", "ㄊㄨㄥ", "ㄋㄨㄥ", "ㄌㄨㄥ", "ㄩㄝ", "ㄐㄩㄝ", "ㄑㄩㄝ", "ㄒㄩㄝ", "ㄋㄩㄝ", "ㄌㄩㄝ", "ㄩㄢ", "ㄐㄩㄢ", "ㄑㄩㄢ", "ㄒㄩㄢ", "ㄌㄩㄢ", "ㄩㄣ",
		"ㄐㄩㄣ", "ㄑㄩㄣ", "ㄒㄩㄣ", "ㄌㄩㄣ", "ㄩㄥ", "ㄐㄩㄥ", "ㄑㄩㄥ", "ㄒㄩㄥ", "ㄦ", "ㄧㄛ", "ㄈㄨㄥ", "ㄈㄧㄠ" };

const char *PINYIN_TABLE[NUM_PINYIN] = { "a", "ai", "an", "ang", "ao", "ba", "bai", "ban", "bang", "bao",			// 1
		"bei", "ben", "beng", "bi", "bian", "biao", "bie", "bin", "bing", "bo",		   		// 2
		"bu", "ca", "cai", "can", "cang", "cao", "ce", "cen", "ceng", "cha",				// 3
		"chai", "chan", "chang", "chao", "che", "chen", "cheng", "chi", "chong", "chou",	// 4
		"chu", "chua", "chuai", "chuan", "chuang", "chui", "chun", "chuo", "ci", "cong",	// 5
		"cou", "cu", "cuan", "cui", "cun", "cuo", "da", "dai", "dan", "dang",				// 6
		"dao", "de", "dei", "den", "deng", "di", "dia", "dian", "diao", "die",				// 7
		"ding", "diu", "dong", "dou", "du", "duan", "dui", "dun", "duo", "e",				// 8
		"E", "ei", "en", "eng", "er", "fa", "fan", "fang", "fei", "fen",					// 9
		"feng", "fo", "fou", "fu", "ga", "gai", "gan", "gang", "gao", "ge",					// 10
		"gei", "gen", "geng", "gong", "gou", "gu", "gua", "guai", "guan", "guang",			// 11
		"gui", "gun", "guo", "ha", "hai", "han", "hang", "hao", "he", "hei",				// 12
		"hen", "heng", "hm", "hng", "hong", "hou", "hu", "hua", "huai", "huan",				// 13
		"huang", "hui", "hun", "huo", "ji", "jia", "jian", "jiang", "jiao", "jie",			// 14
		"jin", "jing", "jiong", "jiu", "ju", "juan", "jue", "jun", "ka", "kai",				// 15
		"kan", "kang", "kao", "ke", "kei", "ken", "keng", "kong", "kou", "ku",  			// 16
		"kua", "kuai", "kuan", "kuang", "kui", "kun", "kuo", "la", "lai", "lan",			// 17
		"lang", "lao", "le", "lei", "leng", "li", "lia", "lian", "liang", "liao",			// 18
		"lie", "lin", "ling", "liu", "lo", "long", "lou", "lu", "lv", "luan",				// 19
		"lve", "lun", "luo", "m", "ma", "mai", "man", "mang", "mao", "me",					// 20
		"mei", "men", "meng", "mi", "mian", "miao", "mie", "min", "ming", "miu",			// 21
		"mo", "mou", "mu", "n", "na", "nai", "nan", "nang", "nao", "ne",					// 22
		"nei", "nen", "neng", "ng", "ni", "nia", "nian", "niang", "niao", "nie",			// 23
		"nin", "ning", "niu", "nong", "nou", "nu", "nv", "nuan", "nve", "nuo",				// 24
		"o", "ou", "pa", "pai", "pan", "pang", "pao", "pei", "pen", "peng",					// 25
		"pi", "pian", "piao", "pie", "pin", "ping", "po", "pou", "pu", "qi",				// 26
		"qia", "qian", "qiang", "qiao", "qie", "qin", "qing", "qiong", "qiu", "qu",			// 27
		"quan", "que", "qun", "ran", "rang", "rao", "re", "ren", "reng", "ri",				// 28
		"rong", "rou", "ru", "rua", "ruan", "rui", "run", "ruo", "sa", "sai",				// 29
		"san", "sang", "sao", "se", "sen", "seng", "sha", "shai", "shan", "shang",			// 30
		"shao", "she", "shei", "shen", "sheng", "shi", "shou", "shu", "shua", "shuai",		// 31
		"shuan", "shuang", "shui", "shun", "shuo", "si", "song", "sou", "su", "suan",		// 32
		"sui", "sun", "suo", "ta", "tai", "tan", "tang", "tao", "te", "teng",				// 33
		"ti", "tian", "tiao", "tie", "ting", "tong", "tou", "tu", "tuan", "tui",			// 34
		"tun", "tuo", "wa", "wai", "wan", "wang", "wei", "wen", "weng", "wo",				// 35
		"wu", "xi", "xia", "xian", "xiang", "xiao", "xie", "xin", "xing", "xiong",			// 36
		"xiu", "xu", "xuan", "xue", "xun", "ya", "yan", "yang", "yao", "ye",				// 37
		"yi", "yin", "ying", "yo", "yong", "you", "yu", "yuan", "yue", "yun",				// 38
		"za", "zai", "zan", "zang", "zao", "ze", "zei", "zen", "zeng", "zha",				// 39
		"zhai", "zhan", "zhang", "zhao", "zhe", "zhei", "zhen", "zheng", "zhi", "zhong",	// 40
		"zhou", "zhu", "zhua", "zhuai", "zhuan", "zhuang", "zhui", "zhun", "zhuo", "zi",	// 41
		"zong", "zou", "zu", "zuan", "zui", "zun", "zuo"									// 42
		};

const char *P2P_TABLE[NUM_PINYIN] =		// pinyin2phone
		{ "ㄚ", "ㄞ", "ㄢ", "ㄤ", "ㄠ", "ㄅㄚ", "ㄅㄞ", "ㄅㄢ", "ㄅㄤ", "ㄅㄠ",							// 1
				"ㄅㄟ", "ㄅㄣ", "ㄅㄥ", "ㄅㄧ", "ㄅㄧㄢ", "ㄅㄧㄠ", "ㄅㄧㄝ", "ㄅㄧㄣ", "ㄅㄧㄥ", "ㄅㄛ",		// 2
				"ㄅㄨ", "ㄘㄚ", "ㄘㄞ", "ㄘㄢ", "ㄘㄤ", "ㄘㄠ", "ㄘㄜ", "ㄘㄣ", "ㄘㄥ", "ㄔㄚ",					// 3
				"ㄔㄞ", "ㄔㄢ", "ㄔㄤ", "ㄔㄠ", "ㄔㄜ", "ㄔㄣ", "ㄔㄥ", "ㄔ", "ㄔㄨㄥ", "ㄔㄡ",					// 4
				"ㄔㄨ", "ㄔㄨㄚ", "ㄔㄨㄞ", "ㄔㄨㄢ", "ㄔㄨㄤ", "ㄔㄨㄟ", "ㄔㄨㄣ", "ㄔㄨㄛ", "ㄘ", "ㄘㄨㄥ",	// 5
				"ㄘㄡ", "ㄘㄨ", "ㄘㄨㄢ", "ㄘㄨㄟ", "ㄘㄨㄣ", "ㄘㄨㄛ", "ㄉㄚ", "ㄉㄞ", "ㄉㄢ", "ㄉㄤ",			// 6
				"ㄉㄠ", "ㄉㄜ", "ㄉㄟ", "ㄉㄣ", "ㄉㄥ", "ㄉㄧ", "ㄉㄧㄚ", "ㄉㄧㄢ", "ㄉㄧㄠ", "ㄉㄧㄝ",			// 7
				"ㄉㄧㄥ", "ㄉㄧㄡ", "ㄉㄨㄥ", "ㄉㄡ", "ㄉㄨ", "ㄉㄨㄢ", "ㄉㄨㄟ", "ㄉㄨㄣ", "ㄉㄨㄛ", "ㄜ",		// 8
				"ㄝ", "ㄝ", "ㄣ", "ㄥ", "ㄦ", "ㄈㄚ", "ㄈㄢ", "ㄈㄤ", "ㄈㄟ", "ㄈㄣ",							// 9
				"ㄈㄥ", "ㄈㄛ", "ㄈㄡ", "ㄈㄨ", "ㄍㄚ", "ㄍㄞ", "ㄍㄢ", "ㄍㄤ", "ㄍㄠ", "ㄍㄜ",					// 10
				"ㄍㄟ", "ㄍㄣ", "ㄍㄥ", "ㄍㄨㄥ", "ㄍㄡ", "ㄍㄨ", "ㄍㄨㄚ", "ㄍㄨㄞ", "ㄍㄨㄢ", "ㄍㄨㄤ",		// 11
				"ㄍㄨㄟ", "ㄍㄨㄣ", "ㄍㄨㄛ", "ㄏㄚ", "ㄏㄞ", "ㄏㄢ", "ㄏㄤ", "ㄏㄠ", "ㄏㄜ", "ㄏㄟ",			// 12
				"ㄏㄣ", "ㄏㄥ", "ㄏㄥ", "ㄏㄥ", "ㄏㄨㄥ", "ㄏㄡ", "ㄏㄨ", "ㄏㄨㄚ", "ㄏㄨㄞ", "ㄏㄨㄢ",			// 13
				"ㄏㄨㄤ", "ㄏㄨㄟ", "ㄏㄨㄣ", "ㄏㄨㄛ", "ㄐㄧ", "ㄐㄧㄚ", "ㄐㄧㄢ", "ㄐㄧㄤ", "ㄐㄧㄠ", "ㄐㄧㄝ",			//14
				"ㄐㄧㄣ", "ㄐㄧㄥ", "ㄐㄩㄥ", "ㄐㄧㄡ", "ㄐㄩ", "ㄐㄩㄢ", "ㄐㄩㄝ", "ㄐㄩㄣ", "ㄎㄚ", "ㄎㄞ",	// 15
				"ㄎㄢ", "ㄎㄤ", "ㄎㄠ", "ㄎㄜ", "ㄎㄟ", "ㄎㄣ", "ㄎㄥ", "ㄎㄨㄥ", "ㄎㄡ", "ㄎㄨ",				// 16
				"ㄎㄨㄚ", "ㄎㄨㄞ", "ㄎㄨㄢ", "ㄎㄨㄤ", "ㄎㄨㄟ", "ㄎㄨㄣ", "ㄎㄨㄛ", "ㄌㄚ", "ㄌㄞ", "ㄌㄢ",	// 17
				"ㄌㄤ", "ㄌㄠ", "ㄌㄜ", "ㄌㄟ", "ㄌㄥ", "ㄌㄧ", "ㄌㄧㄚ", "ㄌㄧㄢ", "ㄌㄧㄤ", "ㄌㄧㄠ",			// 18
				"ㄌㄧㄝ", "ㄌㄧㄣ", "ㄌㄧㄥ", "ㄌㄧㄡ", "ㄌㄛ", "ㄌㄨㄥ", "ㄌㄡ", "ㄌㄨ", "ㄌㄩ", "ㄌㄨㄢ",		// 19
				"ㄌㄩㄝ", "ㄌㄨㄣ", "ㄌㄨㄛ", "ㄣ", "ㄇㄚ", "ㄇㄞ", "ㄇㄢ", "ㄇㄤ", "ㄇㄠ", "ㄇㄛ",				// 20
				"ㄇㄟ", "ㄇㄣ", "ㄇㄥ", "ㄇㄧ", "ㄇㄧㄢ", "ㄇㄧㄠ", "ㄇㄧㄝ", "ㄇㄧㄣ", "ㄇㄧㄥ", "ㄇㄧㄡ",		// 21
				"ㄇㄛ", "ㄇㄡ", "ㄇㄨ", "ㄣ", "ㄋㄚ", "ㄋㄞ", "ㄋㄢ", "ㄋㄤ", "ㄋㄠ", "ㄋㄜ",					// 22
				"ㄋㄟ", "ㄋㄣ", "ㄋㄥ", "ㄣ", "ㄋㄧ", "ㄋㄧㄚ", "ㄋㄧㄢ", "ㄋㄧㄤ", "ㄋㄧㄠ", "ㄋㄧㄝ",			// 23
				"ㄋㄧㄣ", "ㄋㄧㄥ", "ㄋㄧㄡ", "ㄋㄨㄥ", "ㄋㄡ", "ㄋㄨ", "ㄋㄩ", "ㄋㄨㄢ", "ㄋㄩㄝ", "ㄋㄨㄛ",	// 24
				"ㄛ", "ㄡ", "ㄆㄚ", "ㄆㄞ", "ㄆㄢ", "ㄆㄤ", "ㄆㄠ", "ㄆㄟ", "ㄆㄣ", "ㄆㄥ",						// 25
				"ㄆㄧ", "ㄆㄧㄢ", "ㄆㄧㄠ", "ㄆㄧㄝ", "ㄆㄧㄣ", "ㄆㄧㄥ", "ㄆㄛ", "ㄆㄡ", "ㄆㄨ", "ㄑㄧ",		// 26
				"ㄑㄧㄚ", "ㄑㄧㄢ", "ㄑㄧㄤ", "ㄑㄧㄠ", "ㄑㄧㄝ", "ㄑㄧㄣ", "ㄑㄧㄥ", "ㄑㄩㄥ", "ㄑㄧㄡ", "ㄑㄩ",		//27
				"ㄑㄩㄢ", "ㄑㄩㄝ", "ㄑㄩㄣ", "ㄖㄢ", "ㄖㄤ", "ㄖㄠ", "ㄖㄜ", "ㄖㄣ", "ㄖㄥ", "ㄖ",				// 28
				"ㄖㄨㄥ", "ㄖㄡ", "ㄖㄨ", "ㄖㄨㄚ", "ㄖㄨㄢ", "ㄖㄨㄟ", "ㄖㄨㄣ", "ㄖㄨㄛ", "ㄙㄚ", "ㄙㄞ",		// 29
				"ㄙㄢ", "ㄙㄤ", "ㄙㄠ", "ㄙㄜ", "ㄙㄣ", "ㄙㄥ", "ㄕㄚ", "ㄕㄞ", "ㄕㄢ", "ㄕㄤ",					// 30
				"ㄕㄠ", "ㄕㄜ", "ㄕㄟ", "ㄕㄣ", "ㄕㄥ", "ㄕ", "ㄕㄡ", "ㄕㄨ", "ㄕㄨㄚ", "ㄕㄨㄞ",				// 31
				"ㄕㄨㄢ", "ㄕㄨㄤ", "ㄕㄨㄟ", "ㄕㄨㄣ", "ㄕㄨㄛ", "ㄙ", "ㄙㄨㄥ", "ㄙㄡ", "ㄙㄨ", "ㄙㄨㄢ",		// 32
				"ㄙㄨㄟ", "ㄙㄨㄣ", "ㄙㄨㄛ", "ㄊㄚ", "ㄊㄞ", "ㄊㄢ", "ㄊㄤ", "ㄊㄠ", "ㄊㄜ", "ㄊㄥ",			// 33
				"ㄊㄧ", "ㄊㄧㄢ", "ㄊㄧㄠ", "ㄊㄧㄝ", "ㄊㄧㄥ", "ㄊㄨㄥ", "ㄊㄡ", "ㄊㄨ", "ㄊㄨㄢ", "ㄊㄨㄟ",	// 34
				"ㄊㄨㄣ", "ㄊㄨㄛ", "ㄨㄚ", "ㄨㄞ", "ㄨㄢ", "ㄨㄤ", "ㄨㄟ", "ㄨㄣ", "ㄨㄥ", "ㄨㄛ",				// 35
				"ㄨ", "ㄒㄧ", "ㄒㄧㄚ", "ㄒㄧㄢ", "ㄒㄧㄤ", "ㄒㄧㄠ", "ㄒㄧㄝ", "ㄒㄧㄣ", "ㄒㄧㄥ", "ㄒㄩㄥ",	// 36
				"ㄒㄧㄡ", "ㄒㄩ", "ㄒㄩㄢ", "ㄒㄩㄝ", "ㄒㄩㄣ", "ㄧㄚ", "ㄧㄢ", "ㄧㄤ", "ㄧㄠ", "ㄧㄝ",			// 37
				"ㄧ", "ㄧㄣ", "ㄧㄥ", "ㄧㄛ", "ㄩㄥ", "ㄧㄡ", "ㄩ", "ㄩㄢ", "ㄩㄝ", "ㄩㄣ",						// 38
				"ㄗㄚ", "ㄗㄞ", "ㄗㄢ", "ㄗㄤ", "ㄗㄠ", "ㄗㄜ", "ㄗㄟ", "ㄗㄣ", "ㄗㄥ", "ㄓㄚ",					// 39
				"ㄓㄞ", "ㄓㄢ", "ㄓㄤ", "ㄓㄠ", "ㄓㄜ", "ㄓㄟ", "ㄓㄣ", "ㄓㄥ", "ㄓ", "ㄓㄨㄥ",					// 40
				"ㄓㄡ", "ㄓㄨ", "ㄓㄨㄚ", "ㄓㄨㄞ", "ㄓㄨㄢ", "ㄓㄨㄤ", "ㄓㄨㄟ", "ㄓㄨㄣ", "ㄓㄨㄛ", "ㄗ",		// 41
				"ㄗㄨㄥ", "ㄗㄡ", "ㄗㄨ", "ㄗㄨㄢ", "ㄗㄨㄟ", "ㄗㄨㄣ", "ㄗㄨㄛ"								// 42
		};

/*
 void GenerateSidPhoneBig5Table()
 {
 char tmp[100];
 for (int i = 0; i < NUM_PHONE; i++)
 {
 SNDID sid = (SNDID)(i*10+1);
 //TRACE("%4d    %s    %s\n", sid, SoundIdToPhoneBig5(sid), PHONE_TABLE[i]);
 sprintf(tmp,"\"%s\",",SoundIdToPhoneBig5(sid));
 TRACE("%-10s",tmp);
 if(i%10==9) TRACE("\n");
 //else TRACE("\t");
 }
 }
 */
//0                   1                   2                   3                   4
//0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
char bigPHONEME[] = "ㄅㄆㄇㄈㄉㄊㄋㄌㄍㄎㄏㄐㄑㄒㄓㄔㄕㄖㄗㄘㄙㄧㄨㄩㄚㄛㄜㄝㄞㄟㄠㄡㄢㄣㄤㄥㄦˉ˙ˊˇˋ";
//^^內碼是A3BC, 不是A140
//char _BIG5[201];

short GetBig5Index(UCHAR *b5)
{
	int index;
	if(b5[0] < 0xA4 || b5[0] > 0xF9)
	{
		char error[100];
		_error("GetBig5Index() error: %s", b5);
		return 0;
	}
	index = (b5[0] - 0xA4) * 157;
	if(b5[1] > 0xA0)
		index += b5[1] - 0x62; //0xA1 - 0x3F =0x62
	else
		index += b5[1] - 0x40;

	return short(index);
}

// 注音轉成發音代碼
SNDID Phone2SID(UCHAR *phone)
{
	//static CMapStringToPtr map;
	static map<const char*, short> map;
	static BOOL bInit = FALSE;
	void* pv = NULL;
	CString cs;
	UCHAR tone, *phone_tmp;

	tone = 1;
	phone_tmp = phone;
	while(*phone != 0)
	{ //去掉空白
		if(((phone[0] == 0xA3) && (phone[1] == 0xBC)) || ((phone[0] == 0xA1) && (phone[1] == 0x40)))
		{
			phone += 2;
			continue;
		}
		if(phone[0] == 0xA3 && phone[1] >= 0xBB && phone[1] <= 0xBF)
		{
			tone = UCHAR(phone[1] - 0xBB);
			if(tone == 0)
				tone = 5;
			phone += 2;
			continue;
		}

		if(phone[0] != 32)
			cs += phone[0];
		++phone;
	}
	if(cs.getLength() >= 2)
	{
		if(tone == 4 && cs.Compare("ㄟ") == 0)
		{ //因為ㄟ4不在發音表中，所以暫時以ㄧㄝ4來表示
			cs = "ㄧㄝ";
		}
		else if(tone == 5 && cs.Compare("ㄇㄜ") == 0)
		{
			cs = "ㄇㄛ"; //麼：This is wrong, but ...
		}

		if(!bInit)
		{
			for(SNDID phonemeId = 0; phonemeId < NUM_PHONE; ++phonemeId)
			{
				map[PHONE_TABLE[phonemeId]] = (phonemeId * 10);
			}
			bInit = TRUE;
		}

		if(map.find(cs) != map.end())
		{
			return map[cs] + tone;
		}
		//	if(map.Lookup(cs, pv))
		//	{ // found it
		//		return int(pv) + tone;
		//	}
	}

	_error("Phone2SId():SD_BAD\r\n");
	return SD_BAD;
}

void pinyin2Phone(CString &pinyin, char *s, int &tone_p)
{
	char *buffer;
	buffer = pinyin.getBuffer(0);
	int len = pinyin.getLength();
//	tone_p = atoi(&buffer[len-1]);
	for(int i = 0; i < NUM_PINYIN; i++)
	{
		if(memcmp(&buffer[0], PINYIN_TABLE[i], len) == 0)
		{
			strcpy(s, P2P_TABLE[i]);
			break;
		}
	}
	switch(tone_p)
	{
	case 1:
		break;
	case 2:
		strcat(s, "ˊ");
		break;
	case 3:
		strcat(s, "ˇ");
		break;
	case 4:
		strcat(s, "ˋ");
		break;
	case 5:
		strcat(s, "˙");
		break;
	default:
		char msg[100];
		_error("Bad Tone in SoundIdToPhoneAsc:%d", tone_p);
		break;
	}
}

//	發音代碼轉成中文注音: s[31]

void SID2Phone(SNDID soundid, char *s)
{
	//static char s[31];

	if(soundid == SD_BAD)
	{
		sprintf(s, "[SD_BAD]");
		//此處只能處理到4105，超過程示會出錯
	}
	else if(soundid <= SD_SYSTEM + TONE5)
	{
		strcpy(s, PHONE_TABLE[soundid / 10]);

		SNDID toneno = SNDID(soundid % 10);
		switch(toneno)
		{
		case 1:
			break;
		case 2:
			strcat(s, "ˊ");
			break;
		case 3:
			strcat(s, "ˇ");
			break;
		case 4:
			strcat(s, "ˋ");
			break;
		case 5:
			strcat(s, "˙");
			break;
		default:
			_error("Bad Tone in SoundIdToPhoneAsc:%d", toneno);
			break;
		}
	}
	else
	{
		sprintf(s, "[SD_BAD]");
	}
	/* else if (soundid < SD_DIGIT)	{
	 sprintf(s, "[SD_UNDEF]");
	 } else if (soundid < SD_PHONEME) {
	 sprintf(s, "[%c]", soundid-SD_DIGIT+'0');
	 } else if (soundid < SD_ENG) {
	 sprintf(s, "[SD_PHONEME %d]", soundid-SD_PHONEME);
	 } else if (soundid < SD_PUNC){
	 sprintf(s, "[%c]", soundid-SD_ENG+'A');
	 } else if (soundid < SD_USER){
	 sprintf(s, "[SD_PUNC %c]", soundid-SD_PUNC);
	 } else sprintf(s, "[SD_USER %d]", soundid);
	 */
}

void SID2InitialFinal(SNDID soundid, char *initial, char *final)
{
	char s[31];

	if(soundid == SD_BAD)
	{
		initial[0] = 0;
		final[0] = 0;
	}
	else if(soundid <= SD_SYSTEM + TONE5)
	{
		strcpy(s, PHONE_TABLE[soundid / 10]);
		initial[0] = s[0];
		initial[1] = s[1];
		initial[2] = 0;
		int pos = ((strstr(bigPHONEME, initial)) - bigPHONEME) / 2;
		if(pos <= 20)
		{
			strcpy(final, s + 2);
		}
		else
		{
			initial[0] = 0;
			strcpy(final, s);
		}
	}
	else
	{
		initial[0] = 0;
		final[0] = 0;
	}
}

void CV_Group(SNDID soundid, char &c_group, char &v_group, char &c_id, char &v_id)
{
	char s[100], *p;

	if(soundid <= SD_SYSTEM + TONE5)
	{
		p = const_cast<char*>(PHONE_TABLE[soundid / 10]);
		strncpy(s, p, 2);
		s[2] = 0;
	}
	else
	{
		_error("CV_GROUP : error sid");
		c_group = v_group = 0;
		c_id = v_id = -1;
		return;
	}
	//0                   1                   2                   3                   4
	//0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//char bigPHONEME[]= "ㄅㄆㄇㄈㄉㄊㄋㄌㄍㄎㄏㄐㄑㄒㄓㄔㄕㄖㄗㄘㄙㄧㄨㄩㄚㄛㄜㄝㄞㄟㄠㄡㄢㄣㄤㄥㄦˉ˙ˊˇˋ";
	int pos = ((strstr(bigPHONEME, s)) - bigPHONEME) / 2;
	c_id = char(pos);

	switch(pos)
	{
	case 2:  		            //ㄇ
	case 6:  		            //ㄋ
	case 7:  		            //ㄌ
	case 17:  		            //ㄖ
		c_group = 2;
		break;
	case 0:  		            //ㄅ
	case 4:  		            //ㄉ
	case 8:  		            //ㄍ
		c_group = 3;
		break;
	case 3:  		            //ㄈ
	case 10:  		            //ㄏ
		c_group = 4;
		break;
	case 1: //ㄆ
	case 5: //ㄊ
	case 9: //ㄎ
		c_group = 5;
		break;
	case 11: //ㄐ
	case 12: //ㄑ
	case 13: //ㄒ
		c_group = 6;
		break;
	case 14: //ㄓ
	case 15: //ㄔ
	case 16: //ㄕ
		c_group = 7;
		break;
	case 18: //ㄗ
	case 19: //ㄘ
	case 20: //ㄙ
		c_group = 8;
		break;
	default:
		c_group = 1;
		break;
	}
	if(c_group != 1)
		p += 2;
	if(p[0] == 0)
	{
		v_id = 0;
		v_group = 8;
		return;
	}
	strncpy(s, p, 2);
	s[2] = 0;
	pos = ((strstr(bigPHONEME, s)) - bigPHONEME) / 2;
	switch(pos)
	{
	case 24: //ㄚ
	case 28: //ㄞ
	case 30: //ㄠ
		v_id = v_group = 1;
		break;
	case 32: //ㄢ
	case 33: //ㄣ
	case 34: //ㄤ
	case 35: //ㄥ
		v_id = v_group = 2;
		break;
	case 25: //ㄛ
	case 31: //ㄡ
		//ㄨ 在底下處理
		v_id = v_group = 3;
		break;
	case 22: //ㄨ
		if(strlen(p) == 2)
			v_id = v_group = 3;
		else
			v_id = v_group = 9;
		break;
	case 21: //ㄧ
	case 23: //ㄩ
		if(strlen(p) == 2)
			v_id = v_group = 4;
		else
			v_id = v_group = 6;
		break;
	case 26: //ㄜ
	case 27: //ㄝ
	case 29: //ㄟ
	case 36: //ㄦ
		v_id = v_group = 5;
		break;
	default:
		_error("V_GROUP : error vowel");
		break;
	}
}

BOOL VoicedType(SNDID soundid)
{
	char s[100];

	if(soundid <= SD_SYSTEM + TONE5)
	{
		strncpy(s, PHONE_TABLE[soundid / 10], 2);
		s[2] = 0;
	}
	else
	{
		_error("VoicedType : error sid");
		return FALSE;
	}
	//0                   1                   2                   3                   4
	//0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//char bigPHONEME[]= "ㄅㄆㄇㄈㄉㄊㄋㄌㄍㄎㄏㄐㄑㄒㄓㄔㄕㄖㄗㄘㄙㄧㄨㄩㄚㄛㄜㄝㄞㄟㄠㄡㄢㄣㄤㄥㄦˉ˙ˊˇˋ";
	int pos = ((strstr(bigPHONEME, s)) - bigPHONEME) / 2;
	if(pos > 20 || pos == 2 || pos == 6 || pos == 7 || pos == 17)
		return TRUE;
	return FALSE;
}

/*
 ///////////////////////////////////////////////////////////////////////////////
 // ㄅㄆ等注音符號配音
 SNDID GetPhonemeSoundId(char *sen)
 {
 StrNCmpSize(2);
 unsigned int num = strlen(bigPHONEME) >> 1;
 char *find = (char *)lfind(sen, bigPHONEME, &num, 2, (CMPFUNC)StrNCmp);
 if (find)
 return SD_PHONEME + find - bigPHONEME;
 return SD_BAD;
 }
 */
///////////////////////////////////////////////////////////////////////////////
/*----------------------------------------------------------------------------
 FindPhoneET("大", 7, phone);         ㄉㄚˋ, ㄉㄞˋ        return value = 2
 return 注音個數
 ----------------------------------------------------------------------------*/
SNDID *ttsPHONETAB = NULL;

int Big52Phone(UCHAR *str, char phone[][9])
{
	int i, num;
	SNDID sid[10];

	num = Big52SID(str, sid);
	for(i = 0; i < num; i++)
	{
		SID2Phone(sid[i], phone[i]);
	}
	return num;
}

//str:中文字big5，如果是陣列，只處理陣列中的第一個中文字
//sid:存放轉換過的sid陣列，至少為sid[7](一定要注意 by JiunFu)
//傳回破音字的發音個數
int Big52SID(UCHAR *str, SNDID *sid)
{
	// 最後存放到SNDID *sid中的是從ttsPHONETAB查出來的
	int i, index, num;

	if(ttsPHONETAB == NULL)
		return 0;

	index = ttsPHONETAB[GetBig5Index(str)];
	if(index < CHINESE_INDEX_NUM)
	{
		sid[0] = index;
		return 1;
	}
	else
	{
		num = ttsPHONETAB[index++];
		for(i = 0; i < num; i++)
		{
			sid[i] = ttsPHONETAB[index++];
		}
		return num;
	}
}
/*
 ////////////////////////////////////////////////////////////////////////////////////////

 char *corFinal = "5TGBYHNUJM8IK,9OL.0P;/-";  //ㄓㄔㄕㄖㄗㄘㄙㄧㄨㄩㄚㄛㄜㄝㄞㄟㄠㄡㄢㄣㄤㄥㄦ
 char *corInit[] =
 {
 "1QZ2WEDCRFV5TGYHN",				  //ㄅㄆㄈㄉㄊㄍㄎㄏㄐㄑㄒㄓㄔㄕㄗㄘㄙ
 "ASXB",								  //ㄇㄋㄌㄖ
 "UJM8IK,9OL.0P;/-"				  //ㄧㄨㄩㄚㄛㄜㄝㄞㄟㄠㄡㄢㄣㄤㄥㄦ
 };

 //找 soundid 的 initial
 char InitOfSoundId(SNDID sid)
 {
 char phasc[16];
 strcpy(phasc, SoundIdToPhoneAsc(sid));
 if (phasc[0] == '[')
 return '+';
 for (int i = 0; i < 3; i++)
 if (phasc[i] != ' ')
 return phasc[i];
 return '+';
 }

 //找 soundid 的 final
 char FinalOfSoundId(SNDID sid)
 {
 char phasc[16];
 strcpy(phasc, SoundIdToPhoneAsc(sid));
 if (phasc[0] == '[')
 return '+';
 for (int i = 2; i >= 0; i--)
 if (phasc[i] != ' ')
 return phasc[i];
 return '+';
 }

 //傳回連音形態, 0 沒有連音, 1 停頓連接, 2, 3 重疊連接
 int CorType(SNDID left, SNDID right, char *corphone)
 {
 char left_final = FinalOfSoundId(left);
 char right_init =	InitOfSoundId(right);

 if (strchr(corFinal, left_final))
 corphone[0] = left_final;
 else
 corphone[0] = '+';

 for (int i = 0; i < 3; i++)
 if (strchr(corInit[i], right_init))
 {
 corphone[1] = right_init;
 if (corphone[0] == '+')
 return 0;
 return i+1;
 }
 corphone[1] = '+';
 return 0;
 }


 // 0,  0..20 ㄅㄆㄇㄈㄉㄊㄋㄌㄍㄎㄏㄐㄑㄒㄓㄔㄕㄖㄗㄘㄙ
 //	1, 21..23 ㄧㄨㄩ
 // 2, 24..36 ㄚㄛㄜㄝㄞㄟㄠㄡㄢㄣㄤㄥㄦ
 // 3, 37..41 ˉ˙ˊˇˋ",
 int PhonemeType(char ch, char &type)
 {
 int pos;
 pos = strcpos(ascDACHEN, tolower(ch));
 if (pos < 0) 	{ type = -1; return -1; }
 if (pos <= 20)	{ type =  0; return pos + 1; }
 if (pos <= 23)	{ type =  1; return pos - 21 + 1; }
 if (pos <= 36)	{ type =  2; return pos - 24 + 1; }
 if (pos <= 41)	{ type =  3; return pos - 37; }
 type = -1;
 return -1;
 }

 void DelSpaceInBig5(char *s)
 {
 char *space = s;

 do {
 space = strstr(space, " ");
 if (!space)										// no space in s
 return;
 if (space-s < 2 || !IsBig5(space-2))	//no chinese before space
 {
 space = FindNextNotChar(space, ' ');
 continue;
 }
 char *nextbig5;
 nextbig5 = FindNextNotChar(space, ' ');
 if (IsBig5(nextbig5))
 {
 int len = strlen(nextbig5);
 memmove(space, nextbig5, len);
 space[len] = 0;
 space += 2;
 }
 else
 space += 1;
 } while (1);
 }

 void ReplaceSomeChar(char *s)
 {
 static char *b5to_replace = "　０１２３４５６７８９ａｂｃｄｅｆｇｈｉｊｋｌｍｎｏｐｑｒｓｔｕｖｗｘｙｚＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯＰＱＲＳＴＵＶＷＸＹＺ";
 static char *b5tochar  = " 0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
 static char *to_replace = "\t\n";
 static char *tochar  = "  ";

 char *p;
 int len = strlen(s);
 for (int i = 0; i < len;)
 {
 p = strchr(to_replace, s[i]);
 if (p)
 s[i] = tochar[p-to_replace];
 i++;
 }
 }

 char *puncSENBIG5 = "，、。．‧；：？！︰﹐﹒﹖）︶〕︺】︼》︾〉」﹂』﹄／";
 char *puncSENENG = ",;:?!)";
 char *puncBIG5 = "，、。．‧；：？！︰…‥﹐﹑﹒·﹔﹕﹖﹗–—（）︵︶｛｝︷︸〔〕︹︺【】︻︼《》︽︾〈〉︿﹀「」﹁﹂『』﹃﹄‘’“”〝〞‵′＃＆＊※§〃○●△▲◎☆★◇◆□■▽▼＜＞～↑↓←→／＼∕﹨";
 char *puncENG  = "!\"#$%&'()*+,-./:;<=>?[\\]^_`{}";

 inline int IsBig5Punc(char *s)
 {
 //TTP
 if (strncmp("ˋ", s, 2) >= 0 && strncmp("　", s, 2) <= 0)    //   �A�譀歈�
 return 2;
 return 0;
 }

 inline int IsEngPunc(char *s)
 {
 if (s[0] != 0 && strchr(puncENG, s[0]))
 return 1;
 return 0;
 }

 int IsPunc(char *s)
 {
 if (IsBig5Punc(s))
 return 2;
 if (IsEngPunc(s))
 return 1;
 return 0;
 }

 char *FindSentencePunc(char *s)
 {
 int len, i;
 len = strlen(s);
 for (i = 0; i < len;)
 {
 if (IsBig5(s+i))
 {
 int k = i;
 while (IsBig5Punc(s+i) && i < len)
 i += 2;
 if (i > k)
 return s+i;
 i += 2;
 }
 else
 {
 // 1. 2. 3. etc treated an end of sentence
 if (strchr("0123456789", s[i]) && s[i+1] == '.')
 return s+i+2;
 // only a '.' is not treated an end of sen. but ". " do
 if (s[i] == '.')
 {
 if (s[i+1] == ' ')
 return s+i+2;
 if (i == len-1) 		// end of line
 return s+i+1;
 }

 int k = i;
 while (IsEngPunc(s+i) && i < len)
 i++;
 if (i > k)
 return s+i;
 i++;
 }
 }
 return NULL;
 }
 */
