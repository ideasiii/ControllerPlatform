/*
 * service.h
 *
 *  Created on: 2016年10月6日
 *      Author: Jugo
 */

#pragma once

/**
 * Semantic service
 * SERVICE_SEMANTIC：對話服務,轉派給語意服務系統 (情境對話)
 * SERVICE_COMMAND:指令服務,轉派給指令服務模組 (放音樂,叫車,點餐......)
 */
#define SERVICE_SEMANTIC		0x00000001
#define SERVICE_COMMAND			0x00000002

/**
 * Word module access status
 */
#define ERR_SUCCESS				0x00000000
#define ERR_INVALID_JSON		0x00000001
#define ERR_NO_MATCH			0x00000002
#define ERR_INVALID_WORD		0x00000003
#define ERR_EXCEPTION			0x00000004
#define ERR_INVALID_PARAMETER	0x00000005
#define ERR_UNKNOW				0x0000000A
