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

class CTextProcess
{
public:
	explicit CTextProcess();
	virtual ~CTextProcess();
	void processTheText(const char *szText);

private:
	void CartPrediction(CString &sentence, CString &strBig5, std::vector<int>& allPWCluster,
			std::vector<int>& allPPCluster);
	void GenerateLabelFile(CStringArray& sequence, const int sBound[], const int wBound[], const int pBound[],
			const int sCount, const int wCount, const int pCount, std::ofstream& csFile, std::ofstream *pcsFile2);
private:
	int* gduration_s; // time cue
	int* gduration_e; // time cue
	int giSftIdx; // shift of cue idx for each sentence. word-based.
};
