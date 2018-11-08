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
class CWord;
class CART;

//typedef struct _SYLLABLE_ITEM_
//{
//	int nCID;
//	std::vector<int> valFeatureLW;
//	int Sen_Length;					// Sentence length
//	int F_PositionInSen;			// Position in sentence (Forward)
//	int B_PositionInSen;			// Position in sentence (Backward)
//	int PositionInWord;				// Position in lexicon word
//	std::vector<int> valFeaturePOS;
//} SYLLABLE_ITEM;
//
//typedef struct _SYLLABLE_ATT_
//{
//	int size;
//	int featureDim;
//	int nCluster;
//	std::vector<SYLLABLE_ITEM> syllable_item;
//} SYLLABLE_ATT;

class CTextProcess
{
public:
	explicit CTextProcess();
	virtual ~CTextProcess();
	void processTheText(const char *szText);

private:
	void loadModel();
	void releaseModel();
	void CartPrediction(CString &sentence, CString &strBig5, std::vector<int>& allPWCluster,
			std::vector<int>& allPPCluster);
	void GenerateLabelFile(CStringArray& sequence, const int sBound[], const int wBound[], const int pBound[],
			const int sCount, const int wCount, const int pCount, std::ofstream& csFile, std::ofstream *pcsFile2);
	int SplitString(CString& input, CString& delimiter, CStringArray& results);

private:
	CART *CartModel;
	CWord *word;
	int* gduration_s; // time cue
	int* gduration_e; // time cue
	int giSftIdx; // shift of cue idx for each sentence. word-based.
};
