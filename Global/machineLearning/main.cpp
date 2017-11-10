/*
 * main.cpp
 *
 *  Created on: 2017年11月10日
 *      Author: Jugo
 */

#include <map>
#include <iostream>
#include <stdio.h>
#include "CPerceptron.h"

using namespace std;

int main(int argc, char* argv[])
{
	// 感知機 Perceptron
	CPerceptron* perceptron = new CPerceptron(2);

	cout << "感知機 Perceptron" << endl;

	map<int, double> mapXi;

	mapXi[0] = 1.6; 	// TTS Pitch
	mapXi[1] = 1.5;		// TTS Speed
//	mapXi[2] = 0.7;
//	mapXi[3] = 0.4;

	for(int i = 0; i < 1000; ++i)
	{
		perceptron->Iterate(mapXi, 1, 1);
	}
	delete perceptron;
}

