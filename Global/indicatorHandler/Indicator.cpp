/*
 * CIndicator.cpp
 *
 *  Created on: 2017年2月23日
 *      Author: jugo
 */

#include <stdio.h>
#include <unistd.h>
#include <iostream>

#include "Indicator.h"

static int mnLoad = 0;
static char const spin_chars[] = "/-\\|";

void _load()
{
	putchar(spin_chars[mnLoad % sizeof(spin_chars)]);
	fflush(stdout);
	putchar('\b');

	++mnLoad;
	if (3 < mnLoad)
		mnLoad = 0;
}

void _load(int nCount)
{
	std::cout << "[";
	std::cout << nCount << "]" << spin_chars[mnLoad % sizeof(spin_chars)] << "\r";
	std::cout.flush();

	++mnLoad;
	if (3 < mnLoad)
		mnLoad = 0;
}

void _spinner(int spin_seconds)
{
	unsigned long i, num_iterations = (spin_seconds * 10);
	for (i = 0; i < num_iterations; ++i)
	{
		putchar(spin_chars[i % sizeof(spin_chars)]);
		fflush(stdout);
		usleep(100000);
		putchar('\b');
	}
}

float progress = 0.0;
int barWidth = 70;
void _progress(int nCount, int nTotal)
{
	std::cout << "[";
	int pos = barWidth * progress;
	for (int i = 0; i < nTotal; ++i)
	{
		if (i < pos)
			std::cout << "=";
		else if (i == pos)
			std::cout << ">";
		else
			std::cout << " ";
	}
	std::cout << "] " << int((nCount / nTotal) * 100) << " %\r";
	std::cout.flush();

	progress += 0.16; // for demonstration only

}
