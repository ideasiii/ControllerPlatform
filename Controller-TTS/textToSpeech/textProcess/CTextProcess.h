/*
 * CTextProcess.h
 *
 *  Created on: 2018年9月28日
 *      Author: Jugo
 */

#pragma once

#include <iostream>
#include <fstream>
#include <vector>

class CString;
class CStringArray;
class CART;
class CWord;
class CConvert;
class WORD_PACKAGE;

class CTextProcess
{
public:
	explicit CTextProcess();
	virtual ~CTextProcess();
	int processTheText(const char *szText, CString &strWavePath);
	void loadModel();
	void dumpWordData();

private:

	void releaseModel();
	void CartPrediction(CString &sentence, CString &strBig5, std::vector<int>& allPWCluster,
			std::vector<int>& allPPCluster, WORD_PACKAGE &wordPackage);
	CString GenerateLabelFile(CStringArray& sequence, const int sBound[], const int wBound[], const int pBound[],
			const int sCount, const int wCount, const int pCount, std::ofstream& csFile, std::ofstream *pcsFile2,
			int *gduration_s, int *gduration_e, int giSftIdx);
	CString Phone2Ph97(char* phone, int tone);
	int Synthesize(const char* szModelName, const char* szWaveName, const char* szLabel);

private:
	CART *CartModel;
	CConvert *convert;
	CWord *word;
};
