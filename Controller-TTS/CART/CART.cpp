#include "CART.h"
#include "CString.h"
#include <math.h>
#include <float.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "common.h"
#include "CCartData.h"
#include <iostream>
#include <fstream>
#include "WordInfo.h"

using namespace std;

CART_NODE::CART_NODE()
{
	caDataArray.clear();
	dim = -1;
	cuiaQuestion.clear();
	dbEntropy = DBL_MAX;
	clu = -1;
	dbProb = 0;
	Lchild = NULL;
	Rchild = NULL;
	a = 0.0;
	b = 0.0;
}

CART_NODE::~CART_NODE()
{
	// 釋放Lchild與Rchild要與CART的deconstructor一起寫
	caDataArray.clear();
	cuiaQuestion.clear();
}

///////////////////////////////////////////////////////////
//  CART
CART::CART()
{
	// 初始化根節點
	cnRoot = new CART_NODE();
	cnRoot2 = new CART_NODE();
}

CART::~CART()
{
	for(int i = 0; i < (int) gnodeArray.size(); i++)
		delete gnodeArray[i];
	for(int i = 0; i < (int) gnodeArray2.size(); i++)
		delete gnodeArray2[i];
}

void CART::TEST(CART_DATA *cdData)
{
	TEST(cdData, cnRoot);
}

void CART::TEST2(CART_DATA *cdData)
{
	TEST2(cdData, cnRoot2);
}

void CART::TEST(CART_DATA *cdData, CART_NODE *cnNode)
{
	bool flag = false;
	if(cnNode->Lchild)
	{
		for(int i = 0; i < (int) cnNode->cuiaQuestion.size(); ++i)
		{
			if(cnNode->cuiaQuestion[i] == cdData->Att_Catagory[cnNode->dim])
			{
				flag = true;
				break;
			}
		}
		if(flag)
		{
			TEST(cdData, cnNode->Lchild);
		}
		else
		{
			TEST(cdData, cnNode->Rchild);
		}
	}
	else
	{
		cdData->clu = cnNode->clu;
//		cout << "cdData->clu: " << cdData->clu << endl;
	}
}

void CART::TEST2(CART_DATA *cdData, CART_NODE *cnNode)
{
	bool flag = false;
	if(cnNode->Lchild)
	{
		for(int i = 0; i < (int) cnNode->cuiaQuestion.size(); ++i)
		{
			if(cnNode->cuiaQuestion[i] == cdData->Att_Catagory[cnNode->dim])
			{
				flag = true;
				break;
			}
		}
		if(flag)
		{
			TEST2(cdData, cnNode->Lchild);
		}
		else
		{
			TEST2(cdData, cnNode->Rchild);
		}
	}
	else
	{
		cdData->clu = cnNode->clu;
//		cout << "cdData->clu: " << cdData->clu << endl;
	}
}

bool CART::DeleteNode(CART_NODE *cnNode)
{
	if(cnNode->Lchild != NULL)
	{
		DeleteNode(cnNode->Lchild);
		delete cnNode->Lchild;
	}
	if(cnNode->Rchild != NULL)
	{
		DeleteNode(cnNode->Rchild);
		delete cnNode->Rchild;
	}

	return true;
}

/**
 *  使用預設的資料
 */
bool CART::LoadCARTModel()
{
	DeleteNode(cnRoot);
	DeleteNode(cnRoot2);
	LoadCARTModel(1, "model/CART_Model.bin");
	LoadCARTModel(2, "model/CART_Model2.bin");
	return true;
}

bool CART::ConstructCART(int nodeID, CART_NODE *pNode)
{
	int lid = nodeID * 2;
	int rid = nodeID * 2 + 1;
	int index;
	CString strTmp = "";

	for(index = 0; index < (int) vcartData.size(); ++index)
	{
		if(vcartData[index].nodeID == lid)           //nodeID, node->clu, node->dim, size
		{
			CART_NODE *node = new CART_NODE();
			node->clu = vcartData[index].clu;
			node->dim = vcartData[index].dim;
			for(int i = 0; i < vcartData[index].size; ++i)
			{
				node->cuiaQuestion.push_back(vcartData[index].cuiaQuestion[i]);
			}
			pNode->Lchild = node;
			//	_log("[CART] ConstructCART Lchild: %d,%d,%d,%d", lid, pNode->Lchild->clu, pNode->Lchild->dim,vcartData[index].size);
			ConstructCART(lid, pNode->Lchild);
			break;
		}
	}

	for(index = 0; index < (int) vcartData.size(); ++index)
	{
		if(vcartData[index].nodeID == rid)
		{
			CART_NODE *node = new CART_NODE();
			node->clu = vcartData[index].clu;
			node->dim = vcartData[index].dim;
			for(int i = 0; i < vcartData[index].size; ++i)
			{
				node->cuiaQuestion.push_back(vcartData[index].cuiaQuestion[i]);
			}
			pNode->Rchild = node;
//			_log("[CART] ConstructCART Rchild: %d,%d,%d,%d", rid, pNode->Rchild->clu, pNode->Rchild->dim,vcartData[index].size);
			ConstructCART(rid, pNode->Rchild);
			break;
		}
	}
	return FALSE;
}

bool CART::ConstructCART2(int nodeID, CART_NODE *pNode)
{
	int lid = nodeID * 2;
	int rid = nodeID * 2 + 1;
	int index;
	CString strTmp = "";

	for(index = 0; index < (int) vcartData2.size(); ++index)
	{
		if(vcartData2[index].nodeID == lid)
		{
			CART_NODE *node = new CART_NODE();
			node->clu = vcartData2[index].clu;
			node->dim = vcartData2[index].dim;
			for(int i = 0; i < vcartData2[index].size; ++i)
			{
				node->cuiaQuestion.push_back(vcartData2[index].cuiaQuestion[i]);
			}
			pNode->Lchild = node;
//			_log("[CART] ConstructCART Lchild: %d,%d,%d,%d", lid, pNode->Lchild->clu, pNode->Lchild->dim,vcartData[index].size);
			ConstructCART(lid, pNode->Lchild);
			break;
		}
	}

	for(index = 0; index < (int) vcartData2.size(); ++index)
	{
		if(vcartData2[index].nodeID == rid)
		{
			CART_NODE *node = new CART_NODE();
			node->clu = vcartData2[index].clu;
			node->dim = vcartData2[index].dim;
			for(int i = 0; i < vcartData2[index].size; ++i)
			{
				node->cuiaQuestion.push_back(vcartData2[index].cuiaQuestion[i]);
			}
			pNode->Rchild = node;
//			_log("[CART] ConstructCART Rchild: %d,%d,%d,%d", rid, pNode->Rchild->clu, pNode->Rchild->dim,vcartData[index].size);
			ConstructCART(rid, pNode->Rchild);
			break;
		}
	}
	return FALSE;
}

/**
 * 讀取外部模組檔案
 */
bool CART::LoadCARTModel(int cartModel, CString csfile)
{

	int i, fend;
	vector<int> nodeIDArray;

	ifstream cf(csfile.getBuffer(), std::ifstream::binary);
	cf.seekg(0, cf.end);
	fend = cf.tellg();
	cf.seekg(0, cf.beg);
	//	if want to see fend _log("CART_Model size=%d", fend); kris

	CString strData = "";
	CString strQues;
	CString strTmp;

	while(cf.tellg() != fend)
	{
		CART_NODE *node = new CART_NODE();
		int nodeID, size;

		cf.read(reinterpret_cast<char *>(&nodeID), sizeof(int));
		cf.read(reinterpret_cast<char *>(&node->clu), sizeof(int));
		cf.read(reinterpret_cast<char *>(&node->dim), sizeof(int));
		cf.read(reinterpret_cast<char *>(&size), sizeof(int));

		strTmp.format("{ %d,%d,%d,%d,(int[])", nodeID, node->clu, node->dim, size);
		strData += strTmp;
		strQues = "{";
		for(i = 0; i < size; ++i)
		{
			unsigned int t;
			cf.read(reinterpret_cast<char *>(&t), sizeof(unsigned int));
			node->cuiaQuestion.push_back(t);

			if(0 == i)
				strTmp.format("%d", t);
			else
				strTmp.format(",%d", t);
			strQues += strTmp;
		}
		strQues += "}";
		strData = strData + strQues + "},";

		nodeIDArray.push_back(nodeID);
	    if (cartModel == 1)
	    	gnodeArray.push_back(node); // for avoiding lose of nodes that are not added to tree
	    if (cartModel == 2)
	    	gnodeArray2.push_back(node); // for avoiding lose of nodes that are not added to tree
	}
	cf.close();

	//_log("CART NODE: %s", strData.getBuffer()); kris
	if(cartModel == 1){
		if(cnRoot != NULL) // cnRoot is assigned a new CART_NODE in constructor
			delete cnRoot;
		cnRoot = gnodeArray[0];
		ConstructCART(1, nodeIDArray, gnodeArray, cnRoot);
	}

	if(cartModel == 2){
		if(cnRoot2 != NULL) // cnRoot is assigned a new CART_NODE in constructor
			delete cnRoot2;
		cnRoot2 = gnodeArray2[0];
		ConstructCART(1, nodeIDArray, gnodeArray2, cnRoot2);
	}
	return true;
}

bool CART::ConstructCART(int nodeID, vector<int>& IDArray, vector<CART_NODE*>& pNodeData, CART_NODE *pNode)
{
	int lid = nodeID * 2;
	int rid = nodeID * 2 + 1;

	int index;
	for(index = 0; index < (int) IDArray.size(); index++)
	{
		if(IDArray[index] == lid)
		{
			pNode->Lchild = pNodeData[index];
			ConstructCART(lid, IDArray, pNodeData, pNode->Lchild);
			break;
		}
	}
	for(index = 0; index < (int) IDArray.size(); index++)
	{
		if(IDArray[index] == rid)
		{
			pNode->Rchild = pNodeData[index];
			ConstructCART(rid, IDArray, pNodeData, pNode->Rchild);
			break;
		}
	}
	return FALSE;
}
