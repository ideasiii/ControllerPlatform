/*
 * main.cpp
 *
 *  Created on: 2017年07月13日
 *      Author: Jugo
 */
#include <unistd.h> 		// for getopt(), close()
#include <stdio.h>          // for printf() and fprintf()
#include <sys/socket.h>     // for socket(), bind(), and connect()
#include <arpa/inet.h>      // for sockaddr_in and inet_ntoa()
#include <stdlib.h>         // for atoi() and exit()
#include <string.h>         // for memset()
#include <fcntl.h>          // for fcntl()
#include <errno.h>
#include <sys/epoll.h>
#include <map>
#include <getopt.h> 		//getopt_long()頭文件位置
#include <string>

using namespace std;

typedef struct EXAMPLE_ONE
{
	unsigned int valid :1;
	string strContain;
	unsigned int set_flag :2;
} example_one;

typedef struct EXAMPLE_TWO
{
	unsigned int valid :1;
	unsigned int set_flag :2;
	string strContain;
} example_two;

int main(int argc, char* argv[])
{
	/***************** 資料結構 排序影響記憶體配置量 *******************/
	example_one one;
	example_two two;

	printf("size of example one: %d\n", (int) sizeof(one));
	printf("size of example two: %d\n", (int) sizeof(two));

	struct status1 {
	    char a : 3;
	    int b : 29;
	};
	printf("size of status1: %d\n", (int) sizeof(status1)); // a + b = 32 bits = 4bytes

	struct status2 {
	    char a : 4;
	    int b : 29;
	};
	printf("size of status2: %d\n", (int) sizeof(status2)); // a + b > 32 bits = 8bytes

	return 0;
}

