/*
 * dic_artist.h
 *
 *  Created on: 2017年4月25日
 *      Author: Jugo
 */

#pragma once

#include "container.h"

static set<string> setArtistEnglishMale;
static set<string> setArtistEnglishFemale;
static set<string> setArtistTaiwan;
static set<string> setArtistHardRock;
static set<string> setArtistHongKong;
static set<string> setArtistGuitarist;

static map<string, set<string> > mapArtistDic = create_map<string, set<string> >("dictionary/artist_en_male.txt",
		setArtistEnglishMale)("dictionary/artist_en_female.txt", setArtistEnglishFemale)("dictionary/artist_tw.txt",
		setArtistTaiwan)("dictionary/artist_en_hard_rock.txt", setArtistHardRock)("dictionary/artist_hk.txt",
		setArtistHongKong)("dictionary/artist_en_guitarist.txt", setArtistGuitarist);

static set<string> setMark = create_set<string>("(Remastered)")("(Live)")("(Deluxe Remaster)")("(Deluxe)");
