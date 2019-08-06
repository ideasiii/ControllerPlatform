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
	int processTheText(TTS_REQ &ttsProcess, CString &strWavePath, CString &strLabelZip, CString &strChineseData);
	void loadModel();
	int loadWordfromHTTP(string url);
	void dumpWordData();
	void dumpWordIndex();
	void dumpPhone();

	void genLabels();
	int giSftIdx ;
	int* gduration_s;
	int* gduration_e;
	void ConcatenateLabel(string outfilename, char* dir, int iSentenceCnt) ;
	string FinalFileTitle;
	CString strInput_gen, strFileTitle_gen;
	map<string, int>idCount;
	vector<string> splitSentence(string &input);

private:
	void releaseModel();
	int CartPrediction(CString &sentence, CString &strBig5, vector<int>& allPWCluster,
			vector<int>& allPPCluster, WORD_PACKAGE &wordPackage);
	CString GenerateLabelFile(CStringArray& sequence, const int sBound[], const int wBound[], const int pBound[],
			const int sCount, const int wCount, const int pCount, ofstream& csFile, ofstream *pcsFile2,
			int *gduration_s, int *gduration_e, int voice_id);
	CString Phone2Ph97(char* phone, int tone);
	int Synthesize(const char* szModelName, const char* szWaveName, const char* szLabel, TTS_REQ &ttsprocess2);
	void WordExchange(CString &strText);
	CString filterLabel(CString fullstr, int voice_id);
	CString filterLabelLine(char* SplitLabel);
	bool initrd();
	bool timeinfo(int* duration_si,int* duration_ei);

private:
	CART *CartModel;
	CConvert *convert;
	CWord *word;
};
