/*
 * main.cpp
 *
 *  Created on: 2016年03月17日
 *      Author: Jugo
 */

#include <list>
#include <string>
#include <iostream>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include "cJSON.h"
#include "JSONObject.h"
#include "JSONArray.h"
#include <unistd.h>
using namespace std;

int main(int argc, char* argv[])
{
	while (1)
	{
		JSONObject *jobjRoot = new JSONObject("{\"key\":10000}");
		if (jobjRoot->isValid())
		{
			printf("JSON Valid\n");
		}
		jobjRoot->release();
		delete jobjRoot;
		jobjRoot = 0;
		sleep(0.3);
	}

	/*
	 printf("==================== JSONObject Write ============================\n");
	 JSONObject jobjRoot;
	 JSONObject jobjCtrl;
	 JSONArray jArray;
	 JSONObject jobjItem;
	 JSONObject jobjItem2;

	 jobjRoot.put("result", 0);
	 jobjCtrl.put("count", 2);

	 jobjItem.put("type", 0);
	 jobjItem.put("value", 0);
	 jArray.add(jobjItem);

	 jobjItem2.put("type", 1);
	 jobjItem2.put("value", 1);
	 jobjItem2.put("ssid", "xxxxxx");
	 jArray.add(jobjItem2);
	 jArray.add("jtest");

	 jobjCtrl.put("list", jArray);

	 jobjRoot.put("control", jobjCtrl);

	 printf("%s\n", jobjRoot.toString().c_str());

	 string strJSON = jobjRoot.toString();

	 jobjRoot.release();

	 printf("==================== JSONObject Read ============================\n");
	 JSONObject rjobjRoot(strJSON);
	 printf("result:%d\n", rjobjRoot.getInt("result"));
	 JSONObject rjobjCtrl(rjobjRoot.getJsonObject("control"));
	 printf("count:%d\n", rjobjCtrl.getInt("count"));
	 JSONArray rjlist(rjobjCtrl.getJsonArray("list"));

	 for (int i = 0; i < 2; ++i)
	 {
	 JSONObject rjitem(rjlist.getJsonObject(i));
	 printf("type:%d\n", rjitem.getInt("type"));
	 printf("value:%d\n", rjitem.getInt("value"));
	 if (!rjitem.isNull("ssid"))
	 {
	 printf("ssid:%s\n", rjitem.getString("ssid").c_str());
	 }
	 }

	 rjobjRoot.release();
	 */
	//========================== cJSON Example ====================================================//
	/* 
	 strJSON =
	 "{\"Scrollable\":[{\"name\": \"Image67818\",\"Width\": 781,\"Offset\": { \"X\":591, \"Y\": 0},\"src\": \"images/13374i67818.jpg\",\"IsVisible\": true},{\"name\": \"Image67854\",\"Width\": 249,\"Offset\": { \"X\":0, \"Y\": 88},\"src\": \"images/13374i67854.png\",\"IsVisible\": false}]}";
	 cJSON * root;
	 string strValue;
	 int nValue;
	 bool bValue;
	 root = cJSON_Parse(strJSON.c_str());

	 if (root)
	 {
	 cJSON *jsonArray = 0;
	 cJSON *item = 0;
	 cJSON *item2 = 0;
	 if (cJSON_GetObjectItem(root, "Scrollable") && (cJSON_Array == cJSON_GetObjectItem(root, "Scrollable")->type))
	 {
	 jsonArray = cJSON_GetObjectItem(root, "Scrollable");
	 if (jsonArray)
	 {
	 int nCount = cJSON_GetArraySize(jsonArray);
	 for (int i = 0; i < nCount; ++i)
	 {
	 item = cJSON_GetArrayItem(jsonArray, i);
	 strValue = cJSON_GetObjectItem(item, "name")->valuestring;
	 printf("name:%s\n", strValue.c_str());
	 nValue = cJSON_GetObjectItem(item, "Width")->valueint;
	 printf("Width:%d\n", nValue);
	 item2 = cJSON_GetObjectItem(item, "Offset");
	 nValue = cJSON_GetObjectItem(item2, "X")->valueint;
	 printf("X:%d\n", nValue);
	 nValue = cJSON_GetObjectItem(item2, "Y")->valueint;
	 printf("Y:%d\n", nValue);
	 strValue = cJSON_GetObjectItem(item, "src")->valuestring;
	 printf("src:%s\n", strValue.c_str());
	 nValue = cJSON_GetObjectItem(item, "IsVisible")->valueint;
	 printf("IsVisible:%d\n", nValue);
	 }
	 }
	 }
	 }
	 else
	 {
	 printf("Invalid JSON Data");
	 }

	 cJSON_Delete(root);

	 cJSON *root = cJSON_CreateObject();
	 cJSON_AddItemToObject(root, "result", cJSON_CreateNumber(0));

	 cJSON *jobjControl = cJSON_CreateObject();
	 cJSON_AddItemToObject(jobjControl, "count", cJSON_CreateNumber(2));

	 cJSON * jlist = cJSON_CreateArray();
	 cJSON *item = cJSON_CreateObject();
	 cJSON_AddItemToObject(item, "type", cJSON_CreateNumber(0));
	 cJSON_AddItemToObject(item, "value", cJSON_CreateNumber(0));
	 cJSON_AddItemToArray(jlist, item);

	 item = cJSON_CreateObject();
	 cJSON_AddItemToObject(item, "type", cJSON_CreateNumber(1));
	 cJSON_AddItemToObject(item, "value", cJSON_CreateNumber(1));
	 cJSON_AddItemToObject(item, "ssid", cJSON_CreateString("xxxxxxx"));
	 cJSON_AddItemToArray(jlist, item);

	 cJSON_AddItemToObject(jobjControl, "list", jlist);

	 cJSON_AddItemToObject(root, "control", jobjControl);

	 char *out = cJSON_Print(root);
	 printf("%s\n", out);
	 free(out);
	 cJSON_Delete(root);
	 */
}

