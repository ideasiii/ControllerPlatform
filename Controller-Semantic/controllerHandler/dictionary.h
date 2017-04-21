/*
 * dictionary.h
 *
 *  Created on: 2017年4月19日
 *      Author: Jugo
 */

#pragma once

#include "create_map.h"
#include <string>
#include <map>
using namespace std;

static map<string, int> mapSubject = create_map<string, int>\
("我", 1)("你", 2)("他", 3)("我們", 1)("你們", 2)("他們", 3);

