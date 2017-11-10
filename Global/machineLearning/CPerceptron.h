/*
 * CPerceptron.h
 *
 *  Created on: 2017年10月31日
 *      Author: jugo
 *  類神經網路-感知機 Perceptron
 *  感知機的輸出信號取決於權重和閥值，因此我們希望透過經驗法則不斷的根據輸入以及期望輸出來修正權重和閥值，透過不斷重複迭代，
 *  讓輸出值逼近我們想要的輸出值。修正的方法通常利用梯度下降法來修正權重。
 */

#pragma once

#include <map>

class CPerceptron
{
public:
	explicit CPerceptron(int dimension);	// 感知機的維度
	virtual ~CPerceptron();

public:
	double getWeight(int nDimension);
	double getBias();
	double Iterate(std::map<int, double> &mapXi, double d, double learningRate);	//一次迭代 ,輸入,期望輸出 回傳本次迭代誤差函數輸出

private:
	double Net(std::map<int, double> &mapWeight, std::map<int, double> &vecXi, double bias);	//神經元狀態
	double Y(double net);	//神經元輸出
	double Sigmoid(double x); //Sigmoid函數,Sigmoid函數輸出介於0~1之間，輸入越大輸出越接近1
	double D(double d, double Y); //誤差
	double E(double D);	//平方誤差

private:
	std::map<int, double> mapWeight;	//神經元權重組態
	double bias; //神經元偏權組態
};
