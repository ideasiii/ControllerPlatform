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
#include <CController.h>
#include <CString.h>

using namespace std;

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
	int processTheText(TTS_REQ &ttsProcess, CString &strWavePath, CString &strLabelZip, CString &strChineseData,
			int count, CString &outputDir);
	void loadModel();
	int loadWordfromHTTP(string url);
	void dumpWordData();
	void dumpWordIndex();
	void dumpPhone();

	void genLabels();
	int giSftIdx;
	int* gduration_s;
	int* gduration_e;
	void ConcatenateLabel(string outfilename, char* dir, int iSentenceCnt);
	string FinalFileTitle;
	CString strInput_gen, strFileTitle_gen;
	map<string, int> idCount;
	int processTheText_EN(TTS_REQ &ttsProcess, CString &strWavePath, CString &strLabelZip, CString &strChineseData,
			int count, CString &outputDir);

private:
	void releaseModel();
	int CartPrediction(CString &sentence, CString &strBig5, vector<int>& allPWCluster, vector<int>& allPPCluster,
			WORD_PACKAGE &wordPackage);
	int Synthesize(const char* szModelName, const char* szWaveName, const char* szLabel, TTS_REQ &ttsprocess2);
	CString GenerateLabelFile(CStringArray& sequence, const int sBound[], const int wBound[], const int pBound[],
			const int sCount, const int wCount, const int pCount, ofstream& csFile, ofstream *pcsFile2,
			int *gduration_s, int *gduration_e, int voice_id);
	CString Phone2Ph97(char* phone, int tone);
	CString filterLabel(CString fullstr, int voice_id);
	CString filterLabelLine(char* SplitLabel);
	bool initrd();
	bool timeinfo(int* duration_si, int* duration_ei);
	int fliteSynthesize(const char* enModelName, const char* enWaveName, const char* enInputName, TTS_REQ &ttsprocess2);
	void WordExchange(CString &strText); // 舊版數字交換 有誤 改用WordExchange2

private:
	CART *CartModel;
	CConvert *convert;
	CWord *word;
};
