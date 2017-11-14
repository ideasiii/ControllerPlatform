/*
 * main.cpp
 *
 *  Created on: 2017年11月10日
 *      Author: Jugo
 */

#include <string>
#include <map>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "CPerceptron.h"

using namespace std;

int main(int argc, char* argv[])
{
	string strEmotion;

	enum
	{
		NORM = 0, ANGRY, HAPPY, SAD, FEAR
	};

	int nCount = 1000;
	int nEmontion = NORM;
	strEmotion = "平常";

	if(2 <= argc)
	{
		nCount = atoi(argv[1]);
	}

	if(3 <= argc)
	{
		nEmontion = atoi(argv[2]);
		if(0 > nEmontion)
			nEmontion = NORM;
		if(4 < nEmontion)
			nEmontion = NORM;
	}

	// 感知機 Perceptron
	CPerceptron* perceptron = new CPerceptron(1);

	cout << "感知機 Perceptron" << endl;

	map<int, double> mapXi;
	map<int, double> mapWi;

	switch(nEmontion)
	{
	case NORM:
		strEmotion = "平常";
		mapXi[0] = 0.8; 	// TTS Pitch
		mapXi[1] = 1.0;		// TTS Speed
		mapXi[2] = 6;		// TTS Volume
		mapWi[0] = 0.5;		// 人臉情緒
		mapWi[1] = 0.5;		// 人臉情緒
		mapWi[2] = 0.5;		// 人臉情緒
		break;
	case ANGRY:
		strEmotion = "生氣";
		mapXi[0] = 0.25; 	// TTS Pitch
		mapXi[1] = 1.5;		// TTS Speed
		mapXi[2] = 12;		// TTS Volume
		mapWi[0] = 0.5;		// 人臉情緒
		mapWi[1] = 0.5;		// 人臉情緒
		mapWi[2] = 0.5;		// 人臉情緒
		break;
	case HAPPY:
		strEmotion = "開心";
		mapXi[0] = 1.35; 	// TTS Pitch
		mapXi[1] = 1.5;		// TTS Speed
		mapXi[2] = 9;		// TTS Volume
		mapWi[0] = 0.5;		// 人臉情緒
		mapWi[1] = 0.5;		// 人臉情緒
		mapWi[2] = 0.5;		// 人臉情緒
		break;
	case SAD:
		strEmotion = "難過";
		mapXi[0] = 0.05; 	// TTS Pitch
		mapXi[1] = 0.75;		// TTS Speed
		mapXi[2] = 7;		// TTS Volume
		mapWi[0] = 0.5;		// 人臉情緒
		mapWi[1] = 0.5;		// 人臉情緒
		mapWi[2] = 0.5;		// 人臉情緒
		break;
	case FEAR:
		strEmotion = "害怕";
		mapXi[0] = 2; 	// TTS Pitch
		mapXi[1] = 1;		// TTS Speed
		mapXi[2] = 7;		// TTS Volume
		mapWi[0] = 0.5;		// 人臉情緒
		mapWi[1] = 0.5;		// 人臉情緒
		mapWi[2] = 0.5;		// 人臉情緒
		break;
	}

	for(int i = 0; i < nCount; ++i)
	{
		perceptron->Iterate(mapXi, mapWi, 0.8, 0.1);
	}
	delete perceptron;

	printf("\n測試筆數 : %d\n", nCount);
	printf("測試情緒 : %s\n", strEmotion.c_str());
}

