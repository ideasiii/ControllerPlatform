/*
 * main.cpp
 *
 *  Created on: 2015年12月14日
 *      Author: Louis Ju
 */
#include <unistd.h> // for getopt(), close()
#include <stdio.h>          // for printf() and fprintf()
#include <sys/socket.h>     // for socket(), bind(), and connect()
#include <arpa/inet.h>      // for sockaddr_in and inet_ntoa()
#include <stdlib.h>         // for atoi() and exit()
#include <string.h>         // for memset()
#include <fcntl.h>          // for fcntl()
#include <errno.h>
#include <sys/epoll.h>
#include <map>
#include "common.h"
#include "packet.h"
#include "CCmpTest.h"
#include "utility.h"
#include "CEvilTest.h"
#include "JSONObject.h"

#define BUFSIZE 		1024
#define BYE				555
#define PRESSURE		444
#define HELP			777
#define IO_PRESSURE		888
#define EVIL			666
#define WORD			2310

using namespace std;

int cmpRequest(int nCommand, CCmpTest *cmpTest);

int main(int argc, char* argv[])
{
	string strIP;
	int nPort = 0;
	char buffer[BUFSIZE];						// For input
	int i;                      				// For loop use
	int running = 1;            				// Main Loop
	int epfd;                  			 		// EPOLL File Descriptor.
	struct epoll_event ev;                     	// Used for EPOLL.
	struct epoll_event events[5];         		// Used for EPOLL.
	int noEvents;               				// EPOLL event number.
	int nService;								// Test what service.....

	static map<string, int> mapCommand = create_map<string, int>\
("evil", EVIL)\
("bye", BYE)\
("help", HELP)\
("pressure",
	PRESSURE)\
("cmp fcm", fcm_id_register_request)\
("cmp fbtoken", facebook_token_client_request)\
("cmp qrcode",
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
("cmp pwset",
	power_port_set_request)\
("cmp auth", authentication_request)\
("cmp bind", bind_request)\
("cmp unbind",
	unbind_request)\
("amx bind", AMX_BIND)\
("amx system on", AMX_SYSTEM_ON)\
("amx control", amx_control_request)\
(
			"amx status", amx_status_request)\
("semantic", semantic_request)("amx status2", 1166)("word",
	semantic_word_request);

	printf("This process is a Controller testing process!.\n");

	if(argc < 3)
	{
		fprintf( stderr, "Usage:  %s <IP> <Remote Port>  ...\n", argv[0]);
		exit(1);
	}

	strIP = argv[1];
	nPort = atoi(argv[2]);
	printf("Connect IP:%s Port:%d.\n", strIP.c_str(), nPort);
	nService = nPort;

	CCmpTest *cmpTest = new CCmpTest();
	if(!cmpTest->connectController(strIP, nPort))
		exit(0);

	CEvilTest *evil = new CEvilTest(strIP.c_str(), nPort);

	epfd = epoll_create(5);

// Add STDIN into the EPOLL set.
	ev.data.fd = STDIN_FILENO;
	ev.events = EPOLLIN | EPOLLET;
	epoll_ctl(epfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev);
	string strInput;

	while(running)
	{
		noEvents = epoll_wait(epfd, events, FD_SETSIZE, -1);
		for(i = 0; i < noEvents; ++i)
		{
			memset(buffer, 0, BUFSIZE);
			fgets(buffer, 1024, stdin);
			strInput = trim(buffer);

			if(BYE == mapCommand[strInput])
			{
				printf("Bye.\n");
				running = 0;
				break;
			}

			if(WORD == nService)
			{
				JSONObject jsonWord;
				jsonWord.put("id", 0);
				jsonWord.put("type", 0);
				jsonWord.put("word", strInput);
				jsonWord.put("total", 0);
				jsonWord.put("number", 0);
				cmpTest->sendRequest(semantic_word_request, jsonWord.toString().c_str());
				jsonWord.release();
			}
			else
			{
				switch(mapCommand[strInput])
				{
				case EVIL:
					evil->start(500);
					break;
				case BYE:
					printf("Bye.\n");
					running = 0;
					break;
				case HELP:
					printf("Test CMP Use:\n");
					for(map<string, int>::iterator i = mapCommand.begin(); i != mapCommand.end(); ++i)
					{
						cout << (*i).first << endl;
					}
					break;
				default:
					cmpRequest(mapCommand[strInput], cmpTest);
					break;
				}

			}
		}
	}

	delete evil;
	delete cmpTest;
	close(epfd);
	return 0;
}

int cmpRequest(int nCommand, CCmpTest *cmpTest)
{
	switch(nCommand)
	{
	case PRESSURE:
		cmpTest->cmpPressure();
		break;
	case IO_PRESSURE:
		cmpTest->ioPressure();
		break;
	case enquire_link_request:
	case initial_request:
	case sign_up_request:
	case access_log_request:
	case rdm_login_request:
	case rdm_operate_request:
	case rdm_logout_request:
	case power_port_state_request:
	case power_port_set_request:
	case authentication_request:
	case bind_request:
	case unbind_request:
	case semantic_request:
	case amx_control_request:
	case amx_status_request:
	case fcm_id_register_request:
	case facebook_token_client_request:
	case smart_building_qrcode_tokn_request:
	case smart_building_appversion_request:
	case smart_building_getmeetingdata_request:
	case smart_building_amx_control_access_request:
	case smart_building_wireless_power_charge_request:
	case semantic_word_request:
	case 1166:
		cmpTest->sendRequest(nCommand, 0);
		break;
	case AMX_BIND:
	case AMX_SYSTEM_ON:
		cmpTest->sendRequestAMX(nCommand);
		break;
	default:
		printf("Unknow command, use help to show valid command.\n");
		break;
	}

	return 1;
}

