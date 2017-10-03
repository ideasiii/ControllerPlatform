/*
 * main.cpp
 *
 *  Created on: 2017年01月05日
 *      Author: Jugo
 */

#include <string>
#include <stdio.h>
#include "CString.h"

using namespace std;

int main()
{
	CString strTest;
	CString strFormat;

	strTest = "我是中文123AaBb!@#$%";

	printf("word: %s\n", strTest.getBuffer());
	printf("length: %d\n", strTest.getLength());
	printf("find 'A': %d\n", strTest.find('A'));
	printf("find \"A\": %d\n", strTest.find("A"));
	strTest.remove('A');
	printf("remove A word: %s\n", strTest.getBuffer());
	strTest = strTest + "回家囉";

	printf("word: %s\n", strTest.getBuffer());
	printf("length: %d\n", strTest.getLength());
	printf("lower word: %s\n", strTest.makeLower().getBuffer());

	strFormat.format("        我是%s今年%d歲      ", "阿捨", 18);
	printf("word: %s\n", strFormat.getBuffer());
	printf("length: %d\n", strFormat.getLength());

	printf("trim word: %s\n", strFormat.trim().getBuffer());
	printf("length: %d\n", strFormat.getLength());

}
