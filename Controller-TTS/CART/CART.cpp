#include "CART.h"
#include "CString.h"
#include <math.h>
#include <float.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "common.h"

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
}

CART::~CART()
{
	for(int i = 0; i < (int) gnodeArray.size(); i++)
		delete gnodeArray[i];
}

void CART::TEST(CART_DATA *cdData)
{
	TEST(cdData, cnRoot);
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

bool CART::LoadCARTModel(CString csfile)
{
	DeleteNode(cnRoot);

	int i, fend;
	vector<CART_NODE*> nodeArray;
	vector<int> nodeIDArray;

	ifstream cf(csfile.getBuffer(), std::ifstream::binary);
	cf.seekg(0, cf.end);
	fend = cf.tellg();
	cf.seekg(0, cf.beg);
	_log("CART_Model size=%d", fend);
	while(cf.tellg() != fend)
	{
		CART_NODE *node = new CART_NODE();
		int nodeID, size;

		cf.read(reinterpret_cast<char *>(&nodeID), sizeof(int));
		cf.read(reinterpret_cast<char *>(&node->clu), sizeof(int));
		cf.read(reinterpret_cast<char *>(&node->dim), sizeof(int));
		cf.read(reinterpret_cast<char *>(&size), sizeof(int));
		_log("[CART] LoadCARTModel: nodeID=%d clu=%d dim=%d size=%d", nodeID, node->clu, node->dim, size);
		for(i = 0; i < size; ++i)
		{
			unsigned int t;
			cf.read(reinterpret_cast<char *>(&t), sizeof(unsigned int));
			node->cuiaQuestion.push_back(t);
			_log("[CART] cuiaQuestion=%d", t);
		}
		nodeIDArray.push_back(nodeID);
		gnodeArray.push_back(node); // for avoiding lose of nodes that are not added to tree

	}
	cf.close();

	if(cnRoot != NULL) // cnRoot is assigned a new CART_NODE in constructor
		delete cnRoot;
	cnRoot = gnodeArray[0];
	ConstructCART(1, nodeIDArray, gnodeArray, cnRoot);
	return true;
}

bool CART::ConstructCART(int nodeID, std::vector<int>& IDArray, std::vector<CART_NODE*>& pNodeData, CART_NODE *pNode)
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
	return false;
}
