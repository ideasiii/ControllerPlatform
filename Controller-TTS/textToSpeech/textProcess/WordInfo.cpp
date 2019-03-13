#include "WordInfo.h"
//似乎缺少 "M", "半百" 的詞性是M
const char* word_attr[] = { "任意詞類", "數字", "姓", "VA11", "VA12", "VA13", "VA2", "VA3", "VA4", "VB11", "VB12", "VB2",
		"VC1", "VC2", "VC31", "VC32", "VC33", "VD1", "VD2", "VE11", "VE12", "VE2", "VF1", "VF2", "VG1", "VG2", "VH11",
		"VH12", "VH13", "VH14", "VH15", "VH16", "VH17", "VH21", "VH22", "VI1", "VI2", "VI3", "VJ1", "VJ2", "VJ3", "VK1",
		"VK2", "VL1", "VL2", "VL3", "VL4", "V_11", "V_12", "V_2", "VP", "VR", "N", "Naa", "Nab", "Nac", "Nad", "Naea",
		"Naeb", "Nba", "Nbc", "Nca", "Ncb", "Ncc", "Ncda", "Ncdb", "Nce", "Ndaaa", "Ndaab", "Ndaac", "Ndaad", "Ndaba",
		"Ndabb", "Ndabc", "Ndabd", "Ndabe", "Ndabf", "Nhaca", "Nhacb", "Ndbb", "Ndc", "Ndda", "Nddb", "Nddc", "Ne",
		"Nfa", "Nfb", "Nfc", "Nfd", "Nfe", "Nff", "Nfg", "Nfh", "Nfi", "Nfzz", "Ng", "Nhaa", "Nhab", "Nhac", "Nhb",
		"Nhc", "NP", "P01", "P02", "P03", "P04", "P05", "P06", "P07", "P08", "P09", "P10", "P11", "P12", "P13", "P14",
		"P15", "P16", "P17", "P18", "P19", "P20", "P21", "P22", "P23", "P24", "P25", "P26", "P27", "P28", "P29", "P30",
		"P31", "P32", "P33", "P34", "P35", "P36", "P37", "P38", "P39", "P40", "P41", "P42", "P43", "P44", "P45", "P46",
		"P47", "P48", "P49", "P50", "P51", "P52", "P53", "P54", "P55", "P56", "P57", "P58", "P59", "P60", "P61", "P62",
		"P63", "P64", "P65", "A", "b", "Caa", "Cab", "Cbaa", "Cbab", "Cbba", "Cbbb", "Cbca", "Cbcb", "De", "Dia", "I",
		"RD", "S", "Str", "T", "Ta", "Tb", "Tc", "Td", "Da", "Dbaa", "Dbab", "Dbb", "Dbc", "Dc", "Dd", "Dfa", "Dfb",
		"Dg", "Dh", "Di", "Dj", "Dk", "V‧地", "M", "*", 0 };

//"Str": 例如「止」 ??
