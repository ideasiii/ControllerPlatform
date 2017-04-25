/*
 * dictionary.h
 *
 *  Created on: 2017年4月19日
 *      Author: Jugo
 */

#pragma once

#include "container.h"

#define SUBJECT_I			1
#define SUBJECT_YOU			2
#define SUBJECT_HE			3

#define VERB_LISTEN			1

static map<string, int> mapSubject = create_map<string, int>\
("我", SUBJECT_I)("你", SUBJECT_YOU)("他", SUBJECT_HE)("我們",
SUBJECT_I)("你們", SUBJECT_YOU)("他們", SUBJECT_HE);

static map<string, int> mapVerb = create_map<string, int>\
("想要聽", VERB_LISTEN)("超想聽", VERB_LISTEN)("想聽", VERB_LISTEN)(
		"要聽", VERB_LISTEN)("聽", VERB_LISTEN)("聽聽", VERB_LISTEN);

