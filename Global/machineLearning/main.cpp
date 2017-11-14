/*
 * main.cpp
 *
 *  Created on: 2017年11月10日
 *      Author: Jugo
 */

#include <map>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "CPerceptron.h"

using namespace std;

int main(int argc, char* argv[])
{
	int nCount = 1000;

	if(2 <= argc)
	{
		nCount = atoi(argv[1]);
	}
	printf("測試筆數 : %d\n", nCount);

	// 感知機 Perceptron
	CPerceptron* perceptron = new CPerceptron(2);

	cout << "感知機 Perceptron" << endl;

	map<int, double> mapXi;

	mapXi[0] = 1.6; 	// TTS Pitch
	mapXi[1] = 1.5;		// TTS Speed
//	mapXi[2] = 0.7;
//	mapXi[3] = 0.4;

	for(int i = 0; i < nCount; ++i)
	{
		perceptron->Iterate(mapXi, 1, 1);
	}
	delete perceptron;
}

