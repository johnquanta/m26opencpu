/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Quectel Co., Ltd. 2013
*
*****************************************************************************/
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   example_fota_http.c
 *
 * Project:
 * --------
 *   OpenCPU
 *
 * Description:
 * ------------
 *   This example demonstrates how to use fota_http function with RIL APIs in OpenCPU.
 *   Input the specified command through any uart port and the result will be 
 *   output through the debug port.
 *   App bin must be put in server.It will be used to upgrade data through the air.
 *
 * Usage:
 * ------
 *   Compile & Run:
 *
 *     Set "C_PREDEF=-D __EXAMPLE_FOTA_HTTP__" in gcc_makefile file. And compile the 
 *     app using "make clean/new".
 *     Download image bin to module to run.
 * 
 *   Operation:
 *  
 *     step 1: you must put your application bin in your server.
 *     step 2: replace the "APP_BIN_URL" with your own .
 *     step 3: input string : start fota=XXXX, XXXX stands for URL.
 *
 *     The URL format for http is:   http://hostname:port/filePath/fileName                                
 *     NOTE:  if ":port" is be ignored, it means the port is http default port(80) 
 *
 *     eg1: http://23.11.67.89/file/xxx.bin 
 *     eg2: http://www.quectel.com:8080/file/xxx.bin
 * Author:
 * -------
 * -------
 *
 *============================================================================
 *             HISTORY
 *----------------------------------------------------------------------------
 * 
 ****************************************************************************/

#include "custom_feature_def.h"
#include "ql_type.h"
#include "ql_trace.h"
#include "ql_uart.h"
#include "ql_error.h"
#include "ql_gprs.h"
#include "fota_main.h"
#include "ql_stdlib.h"
#include "ql_timer.h"

#include "custom_common.h" 
#include "ServerDataHandle.h" 
#include "my_stdlib.h"

#include "fota_by_http.h"

static void Fota_Start( u8 *BoardID );
static bool CallBack_UpgradeState_Ind(Upgrade_State state, s32 fileDLPercent);

foatInfo_t FOTA;

/****************************************************************************
* Define local function
****************************************************************************/

void Fota_DataHandle( u8 *pDataIn )
{
	MY_MEMCPY( &FOTA.UpgradeFWversion, pDataIn + COM_DATA_OFF,      4 );
	MY_MEMCPY( &FOTA.UpgradeIP,        pDataIn + COM_DATA_OFF + 4,  4 );
	MY_MEMCPY( &FOTA.UpgradePort,      pDataIn + COM_DATA_OFF + 8,  2 );
	MY_MEMCPY( &FOTA.UpgradeToken,     pDataIn + COM_DATA_OFF + 10, 2 );

	MY_MEMSET( FOTA.Upgrading, 0, sizeof( FOTA.Upgrading ) );
	MY_MEMCPY( FOTA.Upgrading, UPGRADING_YES, MY_STRLEN( UPGRADING_YES ) );
	MY_DEBUG("Upgrade ip:%u--prot:%u--Token:%u--Upgrading:%s.\r\n", FOTA.UpgradeIP, FOTA.UpgradePort, FOTA.UpgradeToken, FOTA.Upgrading );
}


static void Fota_Start( u8 *BoardID )
{
    char strIP[20];
	ST_GprsConfig apnCfg;
	char URL_Buffer[128];
	
	MY_MEMCPY(apnCfg.apnName,	device.commTaskData.apnName, Ql_strlen(device.commTaskData.apnName));
	MY_MEMCPY(apnCfg.apnUserId, device.commTaskData.apnUser, Ql_strlen(device.commTaskData.apnUser));
	MY_MEMCPY(apnCfg.apnPasswd, device.commTaskData.apnPassword, Ql_strlen(device.commTaskData.apnPassword));

	/* 
	 Ql_strcpy((char *)m_gprsCfg.apnName, (const char *)device.commTaskData.apnName);
	Ql_strcpy((char *)m_gprsCfg.apnUserId, (const char *)device.commTaskData.apnUser);
	Ql_strcpy((char *)m_gprsCfg.apnPasswd, (const char *)device.commTaskData.apnPassword);
	 */


	MY_MEMSET( strIP, 0, sizeof(strIP) );
	MY_SPRINTF( strIP,"%d.%d.%d.%d", (u8)FOTA.UpgradeIP, (u8)(FOTA.UpgradeIP>>8), (u8)(FOTA.UpgradeIP>>16), (u8)(FOTA.UpgradeIP>>24));
 	
	MY_MEMSET( URL_Buffer, 0, sizeof(URL_Buffer) );
    MY_SPRINTF( URL_Buffer,
		        "http://%s:%d/device/upgrade/?id=%s&tk=%d&fl=%s ",
		        strIP, FOTA.UpgradePort, BoardID, FOTA.UpgradeToken, UPGRADE_BIN_NAME
		      );
	MY_DEBUG("FOTA_URL:%s\r\n", URL_Buffer );
	Ql_FOTA_StartUpgrade( (u8 *)URL_Buffer, &apnCfg, CallBack_UpgradeState_Ind );

   
}

static bool CallBack_UpgradeState_Ind(Upgrade_State state, s32 fileDLPercent) 
{ 
    switch(state) 
    { 
		case UP_START: 
			
		case UP_CONNECTING: 
		case UP_CONNECTED: 
		case UP_GETTING_FILE: 
		case UP_GET_FILE_OK: 
			MY_DEBUG("\r\n<-- Fota Upgrading... -->\r\n"); 
			break; 
		case UP_FOTAINITFAIL: 
		case UP_URLDECODEFAIL: 
		case UP_UPGRADFAILED: 
			MY_DEBUG("\r\n<-- Fota Upgrade failed!! -->\r\n"); 

			Soft_Restart();
			tcp_state_set(0);
			FLAG.updataCount=0;
			FLAG.updataing=0;					
			return FALSE; 
			
		case UP_SYSTEM_REBOOT: 
		{ 
			Ql_Debug_Trace("<--system will reboot, and upgrade. -->\r\n"); 
			return TRUE; 
		} 
		default: 
		break; 
    }
}


void Fota_Pro( u8 *BoardID )
{
	if( 0 != MY_STRCMP( FOTA.Upgrading,  UPGRADING_YES )  )
	{
	   return;
	}
	MY_MEMSET( FOTA.Upgrading, 0, sizeof(FOTA.Upgrading)  );
	Close_Socket();


	// Ql_SOC_Close(m_socketid);
	MY_DEBUG( "Start Upgrading...\r\n" );
	Fota_Start( BoardID );
		FLAG.updataing=1;
		LED_STATE=LED_STATE_GPRS_UPDATA;

	
	 // m_tcp_state=STATE_NW_GET_SIMSTATE;
		
}



