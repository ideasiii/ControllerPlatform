#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include "WaveFile.h"

using namespace std;

CWaveFile::CWaveFile()
{
	//初始資料
	total_cues = 0;			//總 cue 數
}

unsigned int CWaveFile::GetTotalCue()
{
	unsigned long totalcues;
	unsigned long data;
	char head[40];
	char shortdata[5];
	char* wavebuffer = NULL;

	ifstream test;

	test.seekg(0, test.beg);
	test.read(reinterpret_cast<char*>(&head), 40);
	test.read(reinterpret_cast<char*>(&data), 4);
//	this->Seek(0, begin);
//	this->Read(&head, 40);			//位址 0
//	this->Read(&data, 4);				//位址 40 , 值為 資料長度

	wavebuffer = new char[data];
	test.read(wavebuffer, data);
//	this->Read(wavebuffer, data);		// read data into wavebuffer
	free(wavebuffer);
	wavebuffer = NULL;

	test.read(shortdata, 4);
//	this->Read(shortdata, 4);			//開始 read 最後 cue 資料
	shortdata[4] = '\0';
	//先檢查是否有 cue 資料

	if (strcmp(shortdata,"cue ")!=0)
	{
//		AfxMessageBox("This file does not have any cue information", MB_OK);
		test.close();
//		this->Close();
//		AfxAbort();
	}else{
		test.seekg(4, test.cur);
		test.read(reinterpret_cast<char*>(&totalcues), 4);
//		this->Seek(4, current);
//		this->Read(&totalcues, 4);		 // how many cues in the wave file
	}
	total_cues = (unsigned int)totalcues;
	return total_cues;
}

void CWaveFile::GetCueParameter(unsigned int *cuestart, unsigned int *cuelength)
{
	unsigned long data;
	//UINT data;
	char head[40];
	char *wavebuffer=NULL;

	ifstream test2;

	test2.seekg(0, test2.beg);
	test2.read(reinterpret_cast<char*>(&head), 40);
	test2.read(reinterpret_cast<char*>(&data), 4);
//	this->Seek(0, begin);
//	this->Read(&head,40);				//位址 0  , 直接 read 40 個 bytes
//	this->Read(&data,4);				//位址 40 , 值為 資料長度
	wavebuffer = new char[data];
	test2.read(wavebuffer, data);
//	this->Read(wavebuffer, data);		//read 聲音資料
	free(wavebuffer);
	wavebuffer=NULL;

	test2.seekg(12, test2.cur);
//	this->Seek(12, current);
	for (unsigned int i = 0; i < total_cues; i++)
	{
		test2.seekg(4, test2.cur);
		test2.read(reinterpret_cast<char*>(&data), 4);
		test2.seekg(16, test2.cur);
//		this->Seek(4, current);
//		this->Read(&data, 4);
//		this->Seek(16, current);
		cuestart[i] = (unsigned int)data;
	}
	test2.seekg(12, test2.cur);
//	this->Seek(12, current);
	for (unsigned int j=0; j<total_cues; j++)
	{
		test2.seekg(12, test2.cur);
		test2.read(reinterpret_cast<char*>(&data), 4);
		test2.seekg(12, test2.cur);
//		this->Seek(12, current);
//		this->Read(&data, 4);
//		this->Seek(12, current);
		cuelength[j] = (unsigned int)data;
	}
}


