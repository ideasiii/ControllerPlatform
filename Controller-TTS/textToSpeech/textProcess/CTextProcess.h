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
#include <CController.h>   //kris add
#include <CString.h>


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
	int processTheText(TTS_REQ &test, CString &strWavePath);   //kris new processthetext call by reference
	void loadModel();
	void dumpWordData();
	void dumpWordIndex();
	void dumpPhone();

	void genLabels();   // kris new test 2019/04/02
	int giSftIdx ; 		// kris new test 2019/04/03
	int* gduration_s;   // kris new test 2019/04/03
	int* gduration_e;   // kris new test 2019/04/03
	void ConcatenateLabel( std::string outfilename, char* dir, int iSentenceCnt ) ; // kris new test 2019/04/09
	std::string FinalFileTitle; // kris new test 2019/04/09
	CString strInput_test, strFileTitle_test; // kris new test 2019/04/12

private:

	void releaseModel();
	int CartPrediction(CString &sentence, CString &strBig5, std::vector<int>& allPWCluster,
			std::vector<int>& allPPCluster, WORD_PACKAGE &wordPackage);
	CString GenerateLabelFile(CStringArray& sequence, const int sBound[], const int wBound[], const int pBound[],
			const int sCount, const int wCount, const int pCount, std::ofstream& csFile, std::ofstream *pcsFile2,
			int *gduration_s, int *gduration_e, int giSftIdx, int voice_id);
	CString Phone2Ph97(char* phone, int tone);
	int Synthesize(const char* szModelName, const char* szWaveName, const char* szLabel, TTS_REQ &test2);
	void WordExchange(CString &strText);
	CString filterLabel(CString fullstr, int voice_id);  // kris filterLabel 2019/03/07
	CString filterLabelLine(char* SplitLabel); // kris filterLabel 2019/03/07
	bool initrd(); //kris initrd 2019/04/02
	bool timeinfo(int* duration_si,int* duration_ei); //kris timeinfo 2019/04/03
private:
	CART *CartModel;
	CConvert *convert;
	CWord *word;
};
