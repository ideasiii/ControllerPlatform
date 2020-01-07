#ifndef WaveFile_h
//#include "stdafx.h"
#include <iostream>
#include <fstream>
#include "CString.h"
#include "LogHandler.h"
using namespace std;
class CWaveFile : public ifstream
{
protected:
	unsigned int total_cues;		//總 cue 數

public:
	CWaveFile();
	unsigned int GetTotalCue();
	void GetCueParameter(unsigned int* cuestart, unsigned int* cuelength);
	unsigned int GetWaveLength();
	void CutWave(CString & csNewFileName, unsigned int& cuestart, unsigned int& cuelength);
};

#endif
