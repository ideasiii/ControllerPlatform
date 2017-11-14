/*
 * CPerceptron.h
 *
 *  Created on: 2017年10月31日
 *      Author: jugo
 *  類神經網路-感知機 Perceptron
 *  感知機的輸出信號取決於權重和閥值，因此我們希望透過經驗法則不斷的根據輸入以及期望輸出來修正權重和閥值，透過不斷重複迭代，
 *  讓輸出值逼近我們想要的輸出值。修正的方法通常利用梯度下降法來修正權重。
 *
 *   x 是輸入信號，w 是感知機的權重，b是偏權值(也稱閥值)，net 是感知機的狀態，Y是感知機的輸出，d是期望的輸出值
 *   net = w1 * x1 + w2 * x2 + b
 *
 *   感知機的狀態=加總所有的(輸入信號*對應權重)，
 *   直觀來說，一顆神經元的狀態取決於輸入信號的強度多寡與信號的重要性(權重)，
 *   不同種的輸入訊號有不同的重要性，而我們要決定的就是重要性的大小
 *
 *   感知機的輸出的關係式可以表示成:
 *   Y= Activation function(net)
 *   Activation function 激活函數是一種非線性函數，用來將感知機的狀態值重新映射出去，
 *   在不同的類神經網路的架構裡有不同的激活函數，常見有sigmoid、ReLU等等
 *
 *   Sigmoid函數輸出介於0~1之間，輸入越大輸出越接近1，這種非線性的現象是在模擬神經元的傳導原則，
 *   當給予的刺激低於一定程度時，輸出電流趨近於0反之趨近於1。
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
	double Iterate(std::map<int, double> &mapXi, std::map<int, double> &mapWi, double d, double learningRate);

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
