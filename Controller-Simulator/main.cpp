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
#include <getopt.h> //getopt_long()頭文件位置
#include "common.h"
#include "packet.h"
#include "CCmpTest.h"
#include "utility.h"
#include "CEvilTest.h"
#include "JSONObject.h"
#include "Command.h"

#define BUFSIZE 		1024

using namespace std;

void options(int argc, char **argv);
int cmpRequest(int nCommand, CCmpTest *cmpTest);
bool mbPressure;

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
	int nRate;
	printf("This process is a Controller testing process!.\n");

	if (argc < 3)
	{
		printf("argc = %d\n", argc);
		fprintf( stderr, "Usage:  %s <IP> <Remote Port> <Option> ...\n", argv[0]);
		exit(1);
	}

	strIP = argv[1];
	nPort = atoi(argv[2]);
	printf("Connect IP:%s Port:%d.\n", strIP.c_str(), nPort);
	nService = nPort;

	mbPressure = false;
	options(argc, argv);
	if (mbPressure)
		nRate = atoi(argv[4]);

	CCmpTest *cmpTest = new CCmpTest();
	if (!cmpTest->connectController(strIP, nPort))
		exit(0);

	CEvilTest *evil = new CEvilTest(strIP.c_str(), nPort);

	epfd = epoll_create(5);

// Add STDIN into the EPOLL set.
	ev.data.fd = STDIN_FILENO;
	ev.events = EPOLLIN | EPOLLET;
	epoll_ctl(epfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev);
	string strInput;

	while (running)
	{
		noEvents = epoll_wait(epfd, events, FD_SETSIZE, -1);
		for (i = 0; i < noEvents; ++i)
		{
			memset(buffer, 0, BUFSIZE);
			fgets(buffer, 1024, stdin);
			strInput = trim(buffer);

			if (BYE == mapCommands[strInput])
			{
				printf("Bye.\n");
				running = 0;
				break;
			}

			if (WORD == nService)
			{
				JSONObject jsonWord;
				jsonWord.put("id", 1);
				jsonWord.put("type", 4);
				jsonWord.put("word", strInput);
				jsonWord.put("total", 0);
				jsonWord.put("number", 0);
				jsonWord.put("device_id", "chihlee");
				if (mbPressure)
				{
					while (1)
					{
						cmpTest->sendRequest(semantic_word_request, jsonWord.toString().c_str());
						sleep(nRate);
					}
				}
				else
					cmpTest->sendRequest(semantic_word_request, jsonWord.toString().c_str());
				jsonWord.release();
			}
			else if (TTS == nService)
			{
				JSONObject jsonWord;
				jsonWord.put("user_id", "1");
				jsonWord.put("voice_id", 1);
				jsonWord.put("emotion", 1);
				jsonWord.put("text", strInput);
				jsonWord.put("fm", "1");
				jsonWord.put("b", "0.0");
				jsonWord.put("r", "1.25");
				jsonWord.put("id", "a1");
				jsonWord.put("total", 2);
				jsonWord.put("sequence_num", 1);
				jsonWord.put("req_type", 0);
				cmpTest->sendRequest(tts_request, jsonWord.toString().c_str());
				jsonWord.release();
			}
			else
			{
				switch (mapCommands[strInput])
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
					for (map<string, int>::iterator i = mapCommands.begin(); i != mapCommands.end(); ++i)
					{
						cout << (*i).first << endl;
					}
					break;
				default:
					cmpRequest(mapCommands[strInput], cmpTest);
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
	switch (nCommand)
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
	case wheelpies_request:
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
	case controller_die_request:
	case tts_request:
	case 1166:
	case 27027:
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

void options(int argc, char **argv)
{
	const char *optstring = "h:p";
	int c, deb, index;
	struct option opts[] = { { "help", no_argument, NULL, 'h' }, { "pressure", required_argument, NULL, 'p' }, {
			"debug", no_argument, &deb, 1 }, { 0, 0, 0, 0 } };
	while ((c = getopt_long(argc, argv, optstring, opts, &index)) != -1)
	{
		switch (c)
		{
		case 'h':
			printf("Option Parameter:\nsimulator <IP> <Port> [Option]\nOption: -p=pressure test\n");
			printf("Usage Command\n");
			for (map<string, int>::iterator i = mapCommands.begin(); i != mapCommands.end(); ++i)
			{
				cout << (*i).first << endl;
			}
			break;
		case 'p':								//-n 或者 --username 指定用戶名
			mbPressure = true;
			//	printf("pressure is %s\n", optarg);
			break;
		case 0:								//flag不为NULL
			printf("debug is %d\n", deb);
			break;
		case '?':								//選項未定義
			printf("?\n");
			break;
		default:
			printf("c is %d\n", c);
			break;
		}
	}

	/*
	 int c;

	 while((c = getopt(argc, argv, "H:h:P:p")) != -1)
	 {
	 switch(c)
	 {
	 case 'H':
	 case 'h':
	 printf("Option Parameter:\nsimulator <IP> <Port> [Option]\nOption: -p=pressure test\n");
	 printf("Usage Command\n");
	 for(map<string, int>::iterator i = mapCommands.begin(); i != mapCommands.end(); ++i)
	 {
	 cout << (*i).first << endl;
	 }
	 break;
	 case 'P':
	 case 'p':
	 mbPressure = true;
	 printf("Set to Pressure\n");
	 break;
	 }
	 }
	 */
}

