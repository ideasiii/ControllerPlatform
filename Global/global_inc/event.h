/*
 * event.h
 *
 *  Created on: 2015年10月19日
 *      Author: Louis Ju
 */

#pragma once

/*
 * event filter
 * will be defined 2xxx
 */
enum EVENT_FILTER
{
	CONTROLLER = 1, SOCKET_SERVER,
};

#define EVENT_FILTER								2000
#define EVENT_FILTER_CONTROLLER						(EVENT_FILTER + CONTROLLER)
#define EVENT_FILTER_SOCKET_SERVER					(EVENT_FILTER + SOCKET_SERVER)

/********************************************************************
 * event command
 * will be defined 1xxx
 */
enum EVENT_COMMAND
{
	EVENT_SERVER_DOMAIN = 1,
	EVENT_SERVER_TCP,
	EVENT_SERVER_UDP,
	EVENT_CLIENT_CONNECT,
	EVENT_CLIENT_DISCONNECT,
	EVENT_SERVER_TCP_AMX,
	EVENT_SERVER_TCP_DEVICE,
	EVENT_SERVER_TCP_CENTER,
	EVENT_SERVER_TCP_MEETING,
	EVENT_CLIENT_TCP_MEETING_AGENT,
	EVENT_CLIENT_CONNECT_AMX,
	EVENT_CLIENT_DISCONNECT_AMX,
	EVENT_CLIENT_CONNECT_DEVICE,
	EVENT_CLIENT_DISCONNECT_DEVICE,
	EVENT_CLIENT_CONNECT_CENTER,
	EVENT_CLIENT_DISCONNECT_CENTER,
	EVENT_CLIENT_CONNECT_MEETING,
	EVENT_CLIENT_DISCONNECT_MEETING,
	EVENT_SERVER_DISCONNECT_MEETING_AGENT

};

#define EVENT_COMMAND														1000
#define EVENT_COMMAND_SOCKET_TCP_RECEIVE									(EVENT_COMMAND + EVENT_SERVER_TCP)
#define EVENT_COMMAND_SOCKET_UDP_RECEIVE									(EVENT_COMMAND + EVENT_SERVER_UDP)
#define EVENT_COMMAND_SOCKET_CLIENT_CONNECT									(EVENT_COMMAND + EVENT_CLIENT_CONNECT)
#define EVENT_COMMAND_SOCKET_CLIENT_DISCONNECT								(EVENT_COMMAND + EVENT_CLIENT_DISCONNECT)
#define EVENT_COMMAND_SOCKET_TCP_AMX_RECEIVE								(EVENT_COMMAND + EVENT_SERVER_TCP_AMX)
#define EVENT_COMMAND_SOCKET_TCP_DEVICE_RECEIVE								(EVENT_COMMAND + EVENT_SERVER_TCP_DEVICE)
#define EVENT_COMMAND_SOCKET_TCP_CENTER_RECEIVE								(EVENT_COMMAND + EVENT_SERVER_TCP_CENTER)
#define EVENT_COMMAND_SOCKET_CLIENT_CONNECT_AMX								(EVENT_COMMAND + EVENT_CLIENT_CONNECT_AMX)
#define EVENT_COMMAND_SOCKET_CLIENT_DISCONNECT_AMX							(EVENT_COMMAND + EVENT_CLIENT_DISCONNECT_AMX)
#define EVENT_COMMAND_SOCKET_CLIENT_CONNECT_DEVICE							(EVENT_COMMAND + EVENT_CLIENT_CONNECT_DEVICE)
#define EVENT_COMMAND_SOCKET_CLIENT_DISCONNECT_DEVICE						(EVENT_COMMAND + EVENT_CLIENT_DISCONNECT_DEVICE)
#define EVENT_COMMAND_SOCKET_CLIENT_CONNECT_CENTER							(EVENT_COMMAND + EVENT_CLIENT_CONNECT_CENTER)
#define EVENT_COMMAND_SOCKET_CLIENT_DISCONNECT_CENTER						(EVENT_COMMAND + EVENT_CLIENT_DISCONNECT_CENTER)
#define EVENT_COMMAND_SOCKET_TCP_MEETING_RECEIVE							(EVENT_COMMAND + EVENT_SERVER_TCP_MEETING)
#define EVENT_COMMAND_SOCKET_CLIENT_CONNECT_MEETING							(EVENT_COMMAND + EVENT_CLIENT_CONNECT_MEETING)
#define EVENT_COMMAND_SOCKET_CLIENT_DISCONNECT_MEETING						(EVENT_COMMAND + EVENT_CLIENT_DISCONNECT_MEETING)

#define EVENT_COMMAND_SOCKET_TCP_MEETING_AGENT_RECEIVE						(EVENT_COMMAND + EVENT_CLIENT_TCP_MEETING_AGENT)
#define EVENT_COMMAND_SOCKET_SERVER_DISCONNECT_MEETING_AGENT				(EVENT_COMMAND + EVENT_SERVER_DISCONNECT_MEETING_AGENT)
