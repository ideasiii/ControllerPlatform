#pragma once

#include <vector>
#include <fstream>
#include <stdio.h>

typedef struct CART_DATA
{
	int clu;					// 所屬類別
	std::vector<unsigned int> Att_Catagory;	// 類別型屬性, 不管在哪個node, 每筆資料這個陣列的長度都要相等
} CartData;

class CString;

class CART_NODE
{
public:
	CART_NODE();
	~CART_NODE();

	std::vector<CART_DATA*> caDataArray;	    // 在這個node中的資料
	// 若dim小於CART_DATA.Att_Catagory.GetSize(), 表是Questoin是Catagory的, 維度是dim
	// 若dim大於CART_DATA.Att_Catagory.GetSize(), 表是Question是Ordered的, 維度是dim-CART_DATA.Att_Catagory.GetSize()
	int dim;									// 此一question是在哪個維度上的
	std::vector<unsigned int> cuiaQuestion;		// 所用的Catagory question
	// Ordered question: a<=x<b
	double a;									// 所用的Ordered question
	double b;									// 所用的Ordered question
	double dbEntropy;							// 亂度, 不純度
	int clu;									// 這個node是屬於哪個類別
	double dbProb;								// 是這個類別的機率值
	CART_NODE *Lchild;							// 左子樹
	CART_NODE *Rchild;							// 右子樹
};

// 關於有類別為-1的問題, 請在輸入CART之前處理掉
// 目前仍在LoadData()中處理
class CART
{
public:
	// constructor and de-constructor
	CART();
	~CART();
	bool LoadCARTModel(CString csfile);
	bool LoadCARTModel();
	CART_NODE *cnRoot;		// for model/CART_Model.bin
	CART_NODE *cnRoot2;		// for model/CART_Model2.bin

	// test
	void TEST(CART_DATA *cdData);
	void TEST2(CART_DATA *cdData);

private:
	bool ConstructCART(int nodeID, CART_NODE *pNode);
	bool ConstructCART2(int nodeID, CART_NODE *pNode);
	bool ConstructCART(int nodeID, std::vector<int>& IDArray, std::vector<CART_NODE*>& pNodeData, CART_NODE *pNode);
	std::vector<CART_NODE*> gnodeArray;
	void TEST(CART_DATA *cdData, CART_NODE *cnNode);
	bool DeleteNode(CART_NODE *cnNode);
};
