/*
 * Command.h
 *
 *  Created on: 2017年6月15日
 *      Author: Jugo
 */

#pragma once

#include <map>
#include "packet.h"

#define BYE				555
#define PRESSURE		444
#define HELP			777
#define IO_PRESSURE		888
#define EVIL			666
#define WORD			2310
#define MONGO			27027

using namespace std;

static map<string, int> mapCommands = create_map<string, int>\
("evil", EVIL)\
("bye", BYE)\
("help", HELP)\
("pressure",
PRESSURE)\
("mongo", MONGO)\
("cmp fcm", fcm_id_register_request)\
("cmp fbtoken", facebook_token_client_request)\
(
		"cmp qrcode",
		smart_building_qrcode_tokn_request)\
("cmp sbversion", smart_building_appversion_request)\
("cmp sbdata",
smart_building_getmeetingdata_request)\
("cmp amxaccess", smart_building_amx_control_access_request)\
("cmp wpc",
smart_building_wireless_power_charge_request)\
("cmp init", initial_request)\
("cmp signup",
sign_up_request)\
("cmp enquire", enquire_link_request)\
("cmp access", access_log_request)\
("rdm login",
rdm_login_request)\
("rdm operate", rdm_operate_request)\
("rdm logout", rdm_logout_request)\
("io", IO_PRESSURE)\
(
		"cmp pwstate", power_port_state_request)\
("cmp auth",
authentication_request)\
("cmp bind", bind_request)\
("cmp unbind", unbind_request)\
("amx bind", AMX_BIND)\
("amx system on",
		AMX_SYSTEM_ON)\
("amx control", amx_control_request)\
("amx status", amx_status_request)\
("semantic",
semantic_request)("amx status2", 1166)("word", semantic_word_request)("die", controller_die_request)("tts",
		tts_request);
