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

#define BUFSIZE 			1024
#define BYE					555
#define PRESSURE			666
#define HELP				777
#define IO_PRESSURE			888


using namespace std;

int main(int argc, char* argv[])
{
	string strIP;
	int nPort = 0;
	char buffer[BUFSIZE];						// For input
	int i;                      								// For loop use
	int running = 1;            						// Main Loop
	int nCommand = 0;							// command

	int epfd;                  			 				// EPOLL File Descriptor.
	struct epoll_event ev;                     // Used for EPOLL.
	struct epoll_event events[5];         // Used for EPOLL.
	int noEvents;               						// EPOLL event number.

	static map<string, int> mapCommand = create_map<string, int>\
("bye", BYE)\
("help", HELP)\
("pressure", PRESSURE)\
(
			"cmp init", initial_request)\
("cmp signup", sign_up_request)\
("cmp enquire", enquire_link_request)\
(
			"cmp access", access_log_request)\
("rdm login", rdm_login_request)\
("rdm operate", rdm_operate_request)\
(
			"rdm logout", rdm_logout_request)\
("io", IO_PRESSURE)\
("cmp pwstate", power_port_state_request)\
("cmp pwset",
	power_port_set_request)\
("cmp auth", authentication_request)\
("cmp bind", bind_request)\
("cmp unbind",
	unbind_request)\
("amx bind", amx_bind_request);

	printf("This process is a Controller testing process!.\n");

	if (argc < 3)
	{
		fprintf( stderr, "Usage:  %s <IP> <Remote Port>  ...\n", argv[0]);
		exit(1);
	}

	strIP = argv[1];
	nPort = atoi(argv[2]);
	printf("Connect IP:%s Port:%d.\n", strIP.c_str(), nPort);

	CCmpTest *cmpTest = new CCmpTest();
	cmpTest->connectController(strIP, nPort);

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

			nCommand = mapCommand[strInput];

			switch (nCommand)
			{
			case BYE:
				printf("Bye.\n");
				running = 0;
				break;
			case HELP:
				printf("Test CMP Use:\n");
				for (map<string, int>::iterator i = mapCommand.begin(); i != mapCommand.end(); ++i)
				{
					cout << (*i).first << endl;
				}
				break;
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
				cmpTest->sendRequest(nCommand);
				break;
			case amx_bind_request:
				cmpTest->sendRequestAMX(nCommand);
				break;
			default:
				printf("Unknow command, use help to show valid command.\n");
				break;
			}
		}
	}

	delete cmpTest;
	close(epfd);
	return 0;
}

