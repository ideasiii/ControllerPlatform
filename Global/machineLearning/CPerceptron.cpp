/*
 * CPerceptron.cpp
 *
 *  Created on: 2017年10月31日
 *      Author: jugo
 */

#include <string>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "CPerceptron.h"

using namespace std;

CPerceptron::CPerceptron(int dimension)
{
	bias = 0;
	for(int i = 0; i < dimension; ++i)
	{
		mapWeight[i] = 1;
	}
}

CPerceptron::~CPerceptron()
{

}

double CPerceptron::getWeight(int nDimension)
{
	return mapWeight[nDimension];
}

double CPerceptron::getBias()
{
	return bias;
}

double CPerceptron::Net(map<int, double> &mapWeight, map<int, double> &mapXi, double bias)
{
	double net = 0;
	for(unsigned int i = 0; i < mapXi.size(); ++i)
	{
		net += (mapXi[i] * mapWeight[i]); //加總(每個神經元權重*輸入)
		printf("神經元權重 %f * 輸入 %f 加總 %f\n", mapWeight[i], mapXi[i], net);
	}
	net += bias; //最後加偏權值
	return net;
}

double CPerceptron::Y(double net)
{
	return Sigmoid(net);
}

double CPerceptron::Sigmoid(double x)
{
	// 自然對數的基數e約為2.718281828459045。
	return (1 / (1 + pow(2.718, (-1 * x))));
}

double CPerceptron::D(double d, double Y)
{
	return d - Y;
}

double CPerceptron::E(double D) //平方誤差
{
	double E = 0.5 * D * D;
	return E;
}

double CPerceptron::Iterate(map<int, double> &mapXi, double d, double learningRate)
{
	//---前向傳遞
	double net = Net(mapWeight, mapXi, bias);
	printf("神經元狀態值 %f\n", net);

	//---Sigmoid
	double dY = Y(net);
	printf("神經元輸出值 %f\n", dY);

	//---誤差計算
	double dD = D(d, dY);
	printf("目標與實際數出差 %f\n", dD);

	//---平方誤差(誤差函數)
	double dE = E(dD);

	////誤差倒傳遞

	//---對每個weight做更新
	for(unsigned int i = 0; i < mapWeight.size(); ++i)
	{
		double deltaWeight = dD * -1 * dY * (1 - dY) * mapXi[i];	   		//weight 修正量
		mapWeight[i] = mapWeight[i] - learningRate * deltaWeight;	    //更新
	}

	//---對bias做更新
	double deltaBias = dD * dY * (1 - dY) * -1;	   //bias 修正量
	bias = bias - learningRate * deltaBias;	       //更新
	printf("神經元偏權組態 %f\n", bias);
	printf("感知機的失誤度量 %f\n", dE);

	return dE;
}

double CPerceptron::Iterate(map<int, double> &mapXi, map<int, double> &mapWi, double d, double learningRate)
{
	//---前向傳遞
	double net = Net(mapWi, mapXi, bias);
	printf("神經元狀態值 %f\n", net);

	//---Sigmoid
	double dY = Y(net);
	printf("神經元輸出值 %f\n", dY);

	//---誤差計算
	double dD = D(d, dY);
	printf("目標與實際數出差 %f\n", dD);

	//---平方誤差(誤差函數)
	double dE = E(dD);

	////誤差倒傳遞

	//---對每個weight做更新
	for(unsigned int i = 0; i < mapWi.size(); ++i)
	{
		double deltaWeight = dD * -1 * dY * (1 - dY) * mapXi[i];	   		//weight 修正量
		mapWi[i] = mapWi[i] - learningRate * deltaWeight;	    //更新
	}

	//---對bias做更新
	double deltaBias = dD * dY * (1 - dY) * -1;	   //bias 修正量
	bias = bias - learningRate * deltaBias;	       //更新
	printf("神經元偏權組態 %f\n", bias);
	printf("感知機的失誤度量 %f\n", dE);

	return dE;
}

