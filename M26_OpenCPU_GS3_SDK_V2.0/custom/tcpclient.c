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
 *   tcpclient.c
 *
 * Project:
 * --------
 *   OpenCPU
 *
 * Description:
 * ------------
 *   This example demonstrates how to establish a TCP connection, when the module 
 *   is used for the client. Input the specified command through any serial port 
 *   and the result will be output through the debug port.We have adopted a timeout 
 *   mechanism,if in the process of connecting socket or getting the TCP socket ACK 
 *   number overtime 90s, the socket will be close and the network will be deactivated.
 *   In most of TCPIP functions,  return -2(QL_SOC_WOULDBLOCK) doesn't indicate failed.
 *   It means app should wait, till the callback function is called.
 *   The app can get the information of success or failure in callback function.
 *   Get more info about return value. Please read the "OPEN_CPU_DGD" document.
 *
 * Usage:
 * ------
 *   Compile & Run:
 *
 *     Set "C_PREDEF=-D __CUSTOM_TCPCLIENT__" in gcc_makefile file. And compile the 
 *     app using "make clean/new".
 *     Download image bin to module to run.
 * 
 *   Operation:
 *            
 *     step1: set APN parameter.
 *            Command: Set_APN_Param=<APN>,<username>,<password>
 *     step2: set server parameter, which is you want to connect.
 *            Command:Set_Srv_Param=<srv ip>,<srv port>
 *
 * Author:
 * -------
 * -------
 *
 *============================================================================
 *             HISTORY
 *----------------------------------------------------------------------------
 * 
 ****************************************************************************/
#ifdef M26__CUSTOM_TCPCLIENT__  
#include "custom_feature_def.h"
#include "ql_stdlib.h"
#include "ql_common.h"
#include "ql_type.h"
#include "ql_trace.h"
#include "ql_error.h"
#include "ql_uart.h"
#include "ql_gprs.h"
#include "ql_socket.h"
#include "ql_timer.h"
#include "ril_sim.h"
#include "ril_network.h"
#include "ril.h"
#include "ril_util.h"
#include "ril_telephony.h"

#include "custom_common.h"
#include "tool.h"
#include "coin_prize.h"
#include "Ee2prom.h"
#include "at_deviceCmd.h"
#include "ServerDataHandle.h" 


#define MSG_ID_USER_DATA                MSG_ID_USER_START+0x100
#define MSG_ID_MUTEX_TEST               MSG_ID_USER_START+0x101
#define MSG_ID_SEMAPHORE_TEST           MSG_ID_USER_START+0x102
#define MSG_ID_GET_ALL_TASK_PRIORITY    MSG_ID_USER_START+0x103
#define MSG_ID_GET_ALL_TASK_REMAINSTACK MSG_ID_USER_START+0x104




/*****************************************************************
* define process state
******************************************************************/
typedef enum{
    STATE_NW_GET_SIMSTATE,
    STATE_NW_QUERY_STATE,
    STATE_GPRS_REGISTER,
    STATE_GPRS_CONFIG,
    STATE_GPRS_ACTIVATE,
    STATE_GPRS_ACTIVATING,
    STATE_GPRS_GET_DNSADDRESS,
    STATE_GPRS_GET_LOCALIP,
    STATE_CHACK_SRVADDR,
    STATE_SOC_REGISTER,
    STATE_SOC_CREATE,
    STATE_SOC_CONNECT,
    STATE_SOC_CONNECTING,
    STATE_SOC_SEND,
    STATE_SOC_SENDING,
    STATE_SOC_ACK,
    STATE_SOC_CLOSE,
    STATE_GPRS_DEACTIVATE,
    STATE_TOTAL_NUM
}Enum_TCPSTATE;
static u8 m_tcp_state = STATE_NW_GET_SIMSTATE;

/*****************************************************************
* UART Param
******************************************************************/
#define SERIAL_RX_BUFFER_LEN  2048
static u8 m_RxBuf_Uart[SERIAL_RX_BUFFER_LEN];

/*****************************************************************
* APN Param
******************************************************************/
static u8 m_apn[10] = "CMMTM";//"cmnet";
static u8 m_userid[10] = "";
static u8 m_passwd[10] = "";

static ST_GprsConfig  m_gprsCfg;
u8 first_to_send=1;





/*****************************************************************
* Server Param
******************************************************************/
#define SRVADDR_BUFFER_LEN  20
#define UPDATA_IP_LEN       50  
#define ONE_RECV_MAX_LEN    50 //接收一次的最大长度。
#define HOST_NAME           "wmq.weimaqi.net" //"www2.weimaqi.net" 
#define HOST_NAME_PORT      9136

static char  UpdataIPbuff[UPDATA_IP_LEN];
static char  m_SrvADDR[SRVADDR_BUFFER_LEN] = HOST_NAME;
static u32   m_SrvPort = HOST_NAME_PORT;

////#define SEND_BUFFER_LEN     2048
////#define RECV_BUFFER_LEN     2048
////static u8 m_send_buf[SEND_BUFFER_LEN];
////static u8 m_recv_buf[RECV_BUFFER_LEN];
static u64 m_nSentLen  = 0;      // Total bytes of number sent data through current socket  

static u8  m_ipaddress[5];  //only save the number of server ip, remove the comma

static s32 m_socketid = -1; 

static s32 m_remain_len = 0;     // record the remaining number of bytes in send buffer.
////static char *m_pCurrentPos = NULL; 
static u8 open_COIN_UP_TIMER_ID_flag=0;


/*****************************************************************
* GPRS and socket callback function
******************************************************************/
void callback_socket_connect(s32 socketId, s32 errCode, void* customParam );
void callback_socket_close(s32 socketId, s32 errCode, void* customParam );
void callback_socket_accept(s32 listenSocketId, s32 errCode, void* customParam );
void callback_socket_read(s32 socketId, s32 errCode, void* customParam );
void callback_socket_write(s32 socketId, s32 errCode, void* customParam );

void Callback_GPRS_Actived(u8 contexId, s32 errCode, void* customParam);
void CallBack_GPRS_Deactived(u8 contextId, s32 errCode, void* customParam );
void Callback_GetIpByName(u8 contexId, u8 requestId, s32 errCode,  u32 ipAddrCnt, u32* ipAddr);


ST_PDPContxt_Callback     callback_gprs_func = 
{
    Callback_GPRS_Actived,
    CallBack_GPRS_Deactived
};
ST_SOC_Callback      callback_soc_func=
{
    callback_socket_connect,
    callback_socket_close,
    callback_socket_accept,
    callback_socket_read,    
    callback_socket_write
};

/*****************************************************************
* uart callback function
******************************************************************/
static void CallBack_UART_Hdlr(Enum_SerialPort port, Enum_UARTEventType msg, bool level, void* customizedPara);

/*****************************************************************
* timer callback function
******************************************************************/
static void Callback_Timer(u32 timerId, void* param);

/*****************************************************************
* other subroutines
******************************************************************/
extern s32 Analyse_Command(u8* src_str,s32 symbol_num,u8 symbol, u8* dest_buf);
static s32 ReadSerialPort(Enum_SerialPort port, /*[out]*/u8* pBuffer, /*[in]*/u32 bufLen);
static void proc_handle(char *pData,s32 len);

static s32 ret;

extern RxTxCfg_t ServerDataBuff;
extern void ServerDataHandle(void);
extern u8 UpdataIP_Handle( char *pIPdataIn, s32 len,  char *pIPaddrOut, u32 *pIPportOut );
extern void Device_Data_Init(void);

void AfterServerReceiveAll(void);
static void SocketSendData(void);
static s32 ATResponse_CREG_LOCATION_Handler(char* line, u32 len, void* userdata);
void GPRS_COPS_Check(void);
static s32 GPRS_COPS_Check_Handler(char* line, u32 len, void* userdata);

u32 GPRS_Loction_Check(void);
s32 SET_CREG(u8 flag);

void tcp_state_set(u8 state)
{
	m_tcp_state=state;

}
void Close_Socket(void)
{
	Ql_SOC_Close(m_socketid);
}

/*
void GPSSend(void);
void send_data_to_server(u8 com,u8 *pData,u16 len);
void _GPRS_SendData(u8 *pData,u16 len);
*/


static int  s_iMutexId = 0;


void Soft_Restart(void)
{

	APP_DEBUG("\r\n<--sofe restart-->\r\n");
	Ql_RIL_SendATCmd("AT+CFUN=0",Ql_strlen("AT+CFUN=0"), NULL, NULL, 0);
	Ql_Sleep(500);
	Ql_RIL_SendATCmd("AT+CFUN=1",Ql_strlen("AT+CFUN=1"), NULL, NULL, 0);
}





void proc_main_task(s32 taskId)
{
    ST_MSG msg;
	u8 firstwirteflag=0;
   //u8 buf[100]={0};
	
    // Register & open UART port
    Ql_UART_Register(UART_PORT1, CallBack_UART_Hdlr, NULL);
    Ql_UART_Register(UART_PORT2, CallBack_UART_Hdlr, NULL);
    Ql_UART_Register(UART_PORT3, CallBack_UART_Hdlr, NULL);
    Ql_UART_Open(UART_PORT1, 115200, FC_NONE);
    Ql_UART_Open(UART_PORT2, 115200, FC_NONE);
    Ql_UART_Open(UART_PORT3, 115200, FC_NONE);

	Ql_memset(&FLAG,0,sizeof(S_FLAG));

	Ql_memset(&handleData,0,sizeof(handleData));

	Ql_memset(&device,0,sizeof(device));

	gpioInit();
	Device_Data_Init();
	eeprom_Init();
    APP_DEBUG("<--OpenCPU: TCP Client.-->\r\n");

	
    //register & start timer 
    Ql_Timer_Register(TCP_TIMER_ID, Callback_Timer, NULL);
    Ql_Timer_Start(TCP_TIMER_ID, TCP_TIMER_PERIOD, TRUE);

    Ql_Timer_Register(TIMEOUT_90S_TIMER_ID, Callback_Timer, NULL);
    timeout_90S_monitor = FALSE;

    Ql_Timer_Register( COIN_IN_TIMER_ID, Callback_CoinInPulseCheck, NULL );
   Ql_Timer_Start( COIN_IN_TIMER_ID, COIN_IN_TIMER_PERIOD, TRUE);
   Ql_Timer_RegisterFast( COIN_UP_TIMER_ID, Callback_CoinUpPulseOut, NULL );

	//Ql_Timer_Register( COIN_UP_TIMER_ID, Callback_CoinUpPulseOut, NULL );
    Ql_Timer_Start( COIN_UP_TIMER_ID, COIN_UP_TIMER_PERIOD, TRUE);

	Ql_Timer_Register(LED, Callback_LED, NULL);
    Ql_Timer_Start(LED, LED_TIMER_PERIOD, TRUE);

	APP_DEBUG("\r\n<--size=%d-->\r\n",sizeof(device)-2*sizeof(Bill_sut));
	
	APP_DEBUG("\r\n<--NEW 11111111 -->\r\n");



	// s_iMutexId = Ql_OS_CreateMutex("proc_Save"); 
    while(TRUE)   
    {
        Ql_OS_GetMessage(&msg);  
        switch(msg.message)
        {
#ifdef __OCPU_RIL_SUPPORT__
        case MSG_ID_RIL_READY:
            APP_DEBUG("<-- RIL is ready -->\r\n");
            Ql_RIL_Initialize();			
			device_ParaLoad(&device);
			device_Init(&device);
            bill_Read();
			Ql_memcpy(&handleData,&device,sizeof(CommTaskData_sut));
			FLAG.Ready=1;

			APP_DEBUG("<--device.commTaskData.id:%s -->\r\n",device.commTaskData.id);
			APP_DEBUG("<--device.commTaskData.password:%s -->\r\n",device.commTaskData.password);
			restart=0;
			APP_DEBUG("restart=%d\r\n",restart);
			if(restart==0)
				handleData.bitFlag=set_bitFor(handleData.bitFlag, BIT_RESTART,FALSE);
			APP_DEBUG("handleData.bitFlag=%d\r\n",handleData.bitFlag);
			FLAG.Linked=0;

            break;
#endif
        default:
            break;
        }
    }
}

static void CallBack_UART_Hdlr(Enum_SerialPort port, Enum_UARTEventType msg, bool level, void* customizedPara)
{
    switch (msg)
    {
    case EVENT_UART_READY_TO_READ:
        {

           s32 totalBytes = ReadSerialPort(port, m_RxBuf_Uart, sizeof(m_RxBuf_Uart));
           if (totalBytes > 0)
           {
               proc_handle((char *)m_RxBuf_Uart,sizeof(m_RxBuf_Uart));
           }
           break;
        }
    case EVENT_UART_READY_TO_WRITE:
        break;
    default:
        break;
    }
}

static s32 ReadSerialPort(Enum_SerialPort port, /*[out]*/u8* pBuffer, /*[in]*/u32 bufLen)
{
    s32 rdLen = 0;
    s32 rdTotalLen = 0;
    if (NULL == pBuffer || 0 == bufLen)
    {
        return -1;
    }
    Ql_memset(pBuffer, 0x0, bufLen);
    while (1)
    {
        rdLen = Ql_UART_Read(port, pBuffer + rdTotalLen, bufLen - rdTotalLen);
        if (rdLen <= 0)  // All data is read out, or Serial Port Error!
        {
            break;
        }
        rdTotalLen += rdLen;
        // Continue to read...
    }
    if (rdLen < 0) // Serial Port Error!
    {
        APP_DEBUG("<--Fail to read from port[%d]-->\r\n", port);
        return -99;
    }
    return rdTotalLen;
}

static void proc_handle(char *pData,s32 len)
{
    char *p = NULL;
	char iRet;
    u8 srvport[10]; 
	//APP_DEBUG("\r\n<pData=%s>\r\n",pData);
	at_cmdProcess(pData);
   
}





static void checkErr_AckNumber(s32 err_code)
{
    if(SOC_INVALID_SOCKET == err_code)
    {
        APP_DEBUG("<-- Invalid socket ID -->\r\n");
    }
    else if(SOC_INVAL == err_code)
    {
        APP_DEBUG("<-- Invalid parameters for ACK number -->\r\n");
    }
    else if(SOC_ERROR == err_code)
    {
        APP_DEBUG("<-- Unspecified error for ACK number -->\r\n");
    }
    else
    {
        // get the socket option successfully
    }
}


u8 CGREG_deniedErr=0;

static void Callback_Timer( u32 timerId, void* param )
{

    static u16 second=0;
	
	
			if(FLAG.updataing)
			return;	
    if (TIMEOUT_90S_TIMER_ID == timerId)
    {
        APP_DEBUG("<--90s time out!!!-->\r\n");
        APP_DEBUG("<-- Close socket.-->\r\n");
        
        Ql_SOC_Close(m_socketid);
        m_socketid = -1;

        m_tcp_state = STATE_GPRS_DEACTIVATE;

        timeout_90S_monitor = FALSE;
    }
    else if (TCP_TIMER_ID == timerId)
    {


		   if(get_bitFor(handleData.bitFlag, BIT_LINK))
		   {
				if(FLAG.deviceSendBackup>0)
				{
				 APP_DEBUG("\r\n<-- FLAG.deviceSendBackup=%d-->\r\n",FLAG.deviceSendBackup);

					FLAG.deviceSendBackup--;
					if(FLAG.deviceSendBackup==0)
					{
						FLAG.ToServerFlag=1;
						
					}
					

				}
	   
			 }

				if(get_bitFor(handleData.bitFlag, BIT_LINK)){//正常连接
		    //
				Device_Heart_Send();

				}


       //if(FLAG.ISupgrade>=2)
       	{
		   if(handleData.linkTime)
		   {
			   handleData.linkTime--;
			   
			   APP_DEBUG("<--handleData.reConntInterval=%d-->\r\n",handleData.linkTime);
			   
			   if(handleData.linkTime==0)
			   {
		   
				 APP_DEBUG("<--handleData.reConntInterval=0,no data is ready-->\r\n");
				 m_tcp_state=STATE_NW_GET_SIMSTATE;
				 Soft_Restart();

				 FLAG.linkfFalut[3]=2;
				 FLAG.linkfFalut[7]=2;
				  handleData.bitFlag = set_bitFor(handleData.bitFlag, BIT_LINK, FALSE);
			   }
		   }


	   }



        //APP_DEBUG("<--...........m_tcp_state=%d..................-->\r\n",m_tcp_state);
        switch (m_tcp_state)
        {
            case STATE_NW_GET_SIMSTATE:
            {
				

				handleData.bitFlag = set_bitFor(handleData.bitFlag, BIT_LINK, FALSE);
                s32 simStat = 0;
				if((LED_STATE!=LED_STATE_NOSIM_ERR)&&(LED_STATE!=LED_STATE_REG_DENIED_ERR))
				{
					LED_STATE=LED_STATE_NO_LINK;

				}
			
				APP_DEBUG("LED_STATE=%d",LED_STATE_NO_LINK);
                RIL_SIM_GetSimState(&simStat);
                if (simStat == SIM_STAT_READY)
                {
                    char SIMinfo[64];
					FLAG.NoSimCardcount=0;
					Ql_memset( SIMinfo, 0, sizeof(SIMinfo) );                   
                    APP_DEBUG("<--SIM card status is normal!-->\r\n");
					
					if( RIL_AT_SUCCESS == RIL_SIM_GetCCID( SIMinfo ) )
					{
					
					    APP_DEBUG("<--SIM card CCID:%s-->\r\n", SIMinfo );
						Ql_memset( handleData.iccid, 0, sizeof(handleData.iccid) );
						StrToHex( handleData.iccid, SIMinfo, Ql_strlen(SIMinfo)  );					
						APP_DEBUG( "<--HEX CCID:%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x-->\r\n",
							       handleData.iccid[0],handleData.iccid[1],handleData.iccid[2],handleData.iccid[3],handleData.iccid[4],
							       handleData.iccid[5],handleData.iccid[6],handleData.iccid[7],handleData.iccid[8],handleData.iccid[9] ); 
                      Ql_memcpy(FLAG.iccid,handleData.iccid,10);
					}
					
					Ql_memset( SIMinfo, 0, sizeof(SIMinfo) ); 
					if( RIL_AT_SUCCESS == RIL_SIM_GetIMSI( SIMinfo ) )
					{
					    APP_DEBUG("<--SIM card IMSI:%s-->\r\n", SIMinfo );
						//Ql_memset( handleData.ismi, 0, sizeof(handleData.ismi) );		
						//StrToHex( handleData.ismi, SIMinfo+2, Ql_strlen(SIMinfo)-2  ); 
						//APP_DEBUG( "<--HEX IMSI:%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x-->\r\n",
						//	       handleData.ismi[0],handleData.ismi[1],handleData.ismi[2],handleData.ismi[3],handleData.ismi[4],
						//	       handleData.ismi[5],handleData.ismi[6],handleData.ismi[7],handleData.ismi[8],handleData.ismi[9] ); 
						Ql_sscanf((const char*)SIMinfo,"%8d%7d",&handleData.ismi0,&handleData.ismi1);
                      
					}
       
					m_tcp_state = STATE_NW_QUERY_STATE;
					
                }
				else
                {


                     if(FLAG.NoSimCardcount++>10)
                     {
                         FLAG.linkfFalut[0]=FLAG.simFalut++;	
						 FLAG.GprsErrCode|=NO_SIM_CARD;
						 LED_STATE=LED_STATE_NOSIM_ERR;
						 FLAG.SIM_error++;
						 FLAG.NoSimCardcount=0;
						 Soft_Restart();
						 //Ql_Reset(0);
			
						 //	Ql_OS_SendMessage (0, MSG_ID_URC_INDICATION_JINGO, SLEEPENABL,0);
					 }
                     
                     APP_DEBUG("<--SIM card status is unnormal!-->\r\n");
                }
                break;
            }        
            case STATE_NW_QUERY_STATE:
            {
                s32 creg = 0;
                s32 cgreg = 0;
				LED_STATE=LED_STATE_NO_LINK;
                //Ql_NW_GetNetworkState(&creg, &cgreg);
             //   if(RIL_AT_SUCCESS==SET_CREG()) 
             	handleData.bitFlag = set_bitFor(handleData.bitFlag, BIT_LINK, FALSE);
               {
                ret = RIL_NW_GetGSMState(&creg);
				
			//	 FLAG.longlat[2]= GPRS_Loction_Check();
				if((creg == NW_STAT_REGISTERED)||(creg == NW_STAT_REGISTERED_ROAMING))
				{                   
				   ret = RIL_NW_GetGPRSState(&cgreg);
				   if((cgreg == NW_STAT_REGISTERED)||(cgreg == NW_STAT_REGISTERED_ROAMING))
				   {
					 m_tcp_state = STATE_GPRS_REGISTER;
				   }
				   else
				   {
				     if(FLAG.NO_FOUND_CGREG++ > 20)
				     {
                       FLAG.NO_FOUND_CGREG=0;
                       FLAG.CGREG_error++;
					   FLAG.linkfFalut[1]= FLAG.REGFalut++;	
				     }
				   
					if(cgreg==NW_STAT_REG_DENIED)
					{
					     
						 if(CGREG_deniedErr++>10)
						 {
						   CGREG_deniedErr=0;
						   LED_STATE=LED_STATE_REG_DENIED_ERR;						 
						 }


					}
					else 
					{

					}
							

				   }

				}
				else
			    {

                  if(FLAG.NO_FOUND_CREG++ >10)
				  {
                    FLAG.NO_FOUND_CREG=0;
                    FLAG.CREG_error++;

				   }
                     
				
				if(creg==NW_STAT_REG_DENIED)
				{
					if(CGREG_deniedErr++>10)
						{
						CGREG_deniedErr=0;
						LED_STATE=LED_STATE_REG_DENIED_ERR;

					 }
							

				}
				else 
				{

				}

				}

				APP_DEBUG("<--creg=%d,cgreg=%d-->\r\n",creg,cgreg);
              }
              
                break;
            }
            case STATE_GPRS_REGISTER:
            {
			  handleData.bitFlag = set_bitFor(handleData.bitFlag, BIT_LINK, FALSE);
              LED_STATE=LED_STATE_NO_LINK;
			  GPRS_COPS_Check();
			  if(RIL_AT_SUCCESS==SET_CREG(2))//打开基站信息 
		       GPRS_Loction_Check();
			   SET_CREG(0);//关闭基站信息 
                ret = Ql_GPRS_Register(0, &callback_gprs_func, NULL);
                if (GPRS_PDP_SUCCESS == ret)
                {
                    APP_DEBUG("<--Register GPRS callback function successfully.-->\r\n");
                    m_tcp_state = STATE_GPRS_CONFIG;
                }else if (GPRS_PDP_ALREADY == ret)
                {
                    APP_DEBUG("<--GPRS callback function has already been registered,ret=%d.-->\r\n",ret);
                    m_tcp_state = STATE_GPRS_CONFIG;
                }else
                {
                    APP_DEBUG("<--Register GPRS callback function failure,ret=%d.-->\r\n",ret);
                }
                break;
            }
            case STATE_GPRS_CONFIG:
            {
		        u32 ber;
					handleData.bitFlag = set_bitFor(handleData.bitFlag, BIT_LINK, FALSE);
				if( RIL_AT_SUCCESS ==RIL_NW_GetSignalQuality(&handleData.rssi,&ber))
			    {
			      FLAG.Rssi=handleData.rssi;
			      handleData.rssi = handleData.rssi/2-100;
			      APP_DEBUG("<--RSSI:%d  ber=%d-->\r\n", FLAG.Rssi,ber );

				}
				if(handleData.commTaskData.serverAll==0)handleData.bitFlag = set_bitFor(handleData.bitFlag, BIT_UPDATA_IP, TRUE);
                Ql_strcpy((char *)m_gprsCfg.apnName, (const char *)device.commTaskData.apnName);
                Ql_strcpy((char *)m_gprsCfg.apnUserId, (const char *)device.commTaskData.apnUser);
				Ql_strcpy((char *)m_gprsCfg.apnPasswd, (const char *)device.commTaskData.apnPassword);


				APP_DEBUG("\r\n<--m_gprsCfg.apnName:%s -->\r\n", m_gprsCfg.apnName );
				APP_DEBUG("\r\n<--m_gprsCfg.apnUserId:%s -->\r\n", m_gprsCfg.apnUserId );
				APP_DEBUG("\r\n<--m_gprsCfg.apnPasswd:%s -->\r\n", m_gprsCfg.apnPasswd );


				m_gprsCfg.authtype = 0;
                ret = Ql_GPRS_Config(0, &m_gprsCfg);
                if (GPRS_PDP_SUCCESS == ret)
                {
                    APP_DEBUG("<--configure GPRS param successfully.-->\r\n");
                }else
                {
                    APP_DEBUG("<--configure GPRS param failure,ret=%d.-->\r\n",ret);
                }
                
                m_tcp_state = STATE_GPRS_ACTIVATE;
                break;
            }
            case STATE_GPRS_ACTIVATE:
            {
                m_tcp_state = STATE_GPRS_ACTIVATING;
					handleData.bitFlag = set_bitFor(handleData.bitFlag, BIT_LINK, FALSE);
                ret = Ql_GPRS_Activate(0);
                if (ret == GPRS_PDP_SUCCESS)
                {
                    APP_DEBUG("<--Activate GPRS successfully.-->\r\n");
                    m_tcp_state = STATE_GPRS_GET_DNSADDRESS;
                }else if (ret == GPRS_PDP_WOULDBLOCK)
                {
                     APP_DEBUG("<--Waiting for the result of GPRS activated.,ret=%d.-->\r\n",ret);
                    //waiting Callback_GPRS_Actived
                }else if (ret == GPRS_PDP_ALREADY)
                {
                    APP_DEBUG("<--GPRS has already been activated,ret=%d.-->\r\n",ret);
                    m_tcp_state = STATE_GPRS_GET_DNSADDRESS;
                }else//error
                {
                    APP_DEBUG("<--Activate GPRS failure,ret=%d.-->\r\n",ret);
                    m_tcp_state = STATE_GPRS_ACTIVATE;
                }
                break;
            }
            case STATE_GPRS_GET_DNSADDRESS:
            {   
            LED_STATE=LED_STATE_NO_LINK;
				handleData.bitFlag = set_bitFor(handleData.bitFlag, BIT_LINK, FALSE);
                u8 primaryAddr[16] = {0};
                u8 bkAddr[16] = {0};
                ret =Ql_GPRS_GetDNSAddress(0, (u32*)primaryAddr,  (u32*)bkAddr);
                if (ret == GPRS_PDP_SUCCESS)
                {
                    APP_DEBUG("<--Get DNS address successfully,primaryAddr=%d.%d.%d.%d,bkAddr=%d.%d.%d.%d-->\r\n",primaryAddr[0],primaryAddr[1],primaryAddr[2],primaryAddr[3],bkAddr[0],bkAddr[1],bkAddr[2],bkAddr[3]);            
                    m_tcp_state = STATE_GPRS_GET_LOCALIP;
                }else
                {
                     APP_DEBUG("<--Get DNS address failure,ret=%d.-->\r\n",ret);
                    m_tcp_state = STATE_GPRS_DEACTIVATE;
                }
                break;
            }
            case STATE_GPRS_GET_LOCALIP:
            {
                u8 ip_addr[5];
					handleData.bitFlag = set_bitFor(handleData.bitFlag, BIT_LINK, FALSE);
                Ql_memset(ip_addr, 0, 5);
                ret = Ql_GPRS_GetLocalIPAddress(0, (u32 *)ip_addr);
                if (ret == GPRS_PDP_SUCCESS)
                {
                    APP_DEBUG("<--Get Local Ip successfully,Local Ip=%d.%d.%d.%d-->\r\n",ip_addr[0],ip_addr[1],ip_addr[2],ip_addr[3]);
                    m_tcp_state = STATE_CHACK_SRVADDR;
                }else
                {
                    APP_DEBUG("<--Get Local Ip failure,ret=%d.-->\r\n",ret);
                }
                break;
            }
            case STATE_CHACK_SRVADDR:
            {
                   LED_STATE=LED_STATE_NO_LINK;
				   handleData.bitFlag = set_bitFor(handleData.bitFlag, BIT_LINK, FALSE);

				if(get_bitFor(handleData.bitFlag, BIT_UPDATA_IP))
			    {

				 Ql_memcpy(m_SrvADDR,HOST_NAME,Ql_strlen(HOST_NAME)); 
				 m_SrvPort = HOST_NAME_PORT;

				}
				else 
					
			    {
					char pTemp1[20]={0};
					u32 ServerIp=0;


					//ServerIp  = 123456789;

					ServerIp  = device.commTaskData.ip[0];
					m_SrvPort = device.commTaskData.port[0];

					Ql_sprintf((char*)pTemp1,"%d.%d.%d.%d",(u8)(ServerIp>>24),(u8)(ServerIp>>16),(u8)(ServerIp>>8),(u8)ServerIp);
					Ql_memcpy(m_SrvADDR,pTemp1,20);
				}

				
                Ql_memset(m_ipaddress,0,5);
				APP_DEBUG("<<<-- m_SrvADDR=%s,m_SrvPort=%d -->>>\r\n",m_SrvADDR, m_SrvPort);
                ret = Ql_IpHelper_ConvertIpAddr(m_SrvADDR, (u32 *)m_ipaddress);
                if(ret == SOC_SUCCESS) // ip address, xxx.xxx.xxx.xxx
                {
                    APP_DEBUG("<--Convert Ip Address successfully,m_ipaddress=%d,%d,%d,%d-->\r\n",m_ipaddress[0],m_ipaddress[1],m_ipaddress[2],m_ipaddress[3]);
                    m_tcp_state = STATE_SOC_REGISTER;
                    
                }
				else  //domain name
                {
                    ret = Ql_IpHelper_GetIPByHostName(0, 0, m_SrvADDR, Callback_GetIpByName);
                    if(ret == SOC_SUCCESS)
                    {
                        APP_DEBUG("<--Get ip by hostname successfully.-->\r\n");
                    }
                    else if(ret == SOC_WOULDBLOCK)
                    {
                        APP_DEBUG("<--Waiting for the result of Getting ip by hostname,ret=%d.-->\r\n",ret);
                        //waiting CallBack_getipbyname
                    }
                    else
                    {
                        APP_DEBUG("<--Get ip by hostname failure:ret=%d-->\r\n",ret);
                        if(ret == SOC_BEARER_FAIL)  
                        {
                             m_tcp_state = STATE_GPRS_DEACTIVATE;
                        }
                        else
                        {
                             m_tcp_state = STATE_GPRS_GET_DNSADDRESS;
                        } 
                    }
                }
                break;
            }
            case STATE_SOC_REGISTER:
            {
            LED_STATE=LED_STATE_NO_LINK;
				handleData.bitFlag = set_bitFor(handleData.bitFlag, BIT_LINK, FALSE);
                ret = Ql_SOC_Register(callback_soc_func, NULL);
                if (SOC_SUCCESS == ret)
                {
                    APP_DEBUG("<--Register socket callback function successfully.-->\r\n");
                    m_tcp_state = STATE_SOC_CREATE;
                }else if (SOC_ALREADY == ret)
                {
                    APP_DEBUG("<--Socket callback function has already been registered,ret=%d.-->\r\n",ret);
                    m_tcp_state = STATE_SOC_CREATE;
                }else
                {
                    APP_DEBUG("<--Register Socket callback function failure,ret=%d.-->\r\n",ret);
                }
                break;
            }
            case STATE_SOC_CREATE:
            {
            LED_STATE=LED_STATE_NO_LINK;
				handleData.bitFlag = set_bitFor(handleData.bitFlag, BIT_LINK, FALSE);
                m_socketid = Ql_SOC_Create(0, SOC_TYPE_TCP);
                if (m_socketid >= 0)
                {
                    APP_DEBUG("<--Create socket id successfully,socketid=%d.-->\r\n",m_socketid);
                    m_tcp_state = STATE_SOC_CONNECT;
                }else
                {
                    APP_DEBUG("<--Create socket id failure,error=%d.-->\r\n",m_socketid);
                }
                break;
            }
            case STATE_SOC_CONNECT:
            {
                LED_STATE=LED_STATE_NO_LINK;
				handleData.bitFlag = set_bitFor(handleData.bitFlag, BIT_LINK, FALSE);
                m_tcp_state = STATE_SOC_CONNECTING;
                ret = Ql_SOC_Connect(m_socketid,(u32) m_ipaddress, m_SrvPort);
                if(ret == SOC_SUCCESS)
                {
                    FLAG.ConnetErrCount=0;
					//LED_STATE=SERVER_LINK;
					//APP_DEBUG("LED_STATE=%d",SERVER_LINK);
                    APP_DEBUG("<--The socket is already connected.-->\r\n");
                    m_tcp_state = STATE_SOC_SEND;
                    
                }else if(ret == SOC_WOULDBLOCK)
                {
                      if (!timeout_90S_monitor)//start timeout monitor
                      {
                        Ql_Timer_Start(TIMEOUT_90S_TIMER_ID, TIMEOUT_90S_PERIOD, FALSE);
                        timeout_90S_monitor = TRUE;
                      }
                      APP_DEBUG("<--Waiting for the result of socket connection,ret=%d.-->\r\n",ret);
                      //waiting CallBack_getipbyname

					 if(FLAG.ConnetErrCount++>4)
                     {
                       FLAG.ConnetErrCount=0;
					   
					   handleData.bitFlag = set_bitFor(handleData.bitFlag, BIT_UPDATA_IP,TRUE);
					 }
                      
                }else //error
                {
                    APP_DEBUG("<--Socket Connect failure,ret=%d.-->\r\n",ret);
                    APP_DEBUG("<-- Close socket.-->\r\n");
                    Ql_SOC_Close(m_socketid);
                    m_socketid = -1;

                     if(FLAG.ConnetErrCount++>3)
                     {
                       FLAG.ConnetErrCount=0;
					   
					   handleData.bitFlag = set_bitFor(handleData.bitFlag, BIT_UPDATA_IP,TRUE);
					 }
					
                    if(ret == SOC_BEARER_FAIL)  
                    {
                        m_tcp_state = STATE_GPRS_DEACTIVATE;
                    }
                    else
                    {
                        m_tcp_state = STATE_GPRS_GET_DNSADDRESS;
                    }
                }
                break;
            }
            case STATE_SOC_SEND:
            {   
				if( ServerDataBuff.txHead >= HAL_TCP_TX_MAX )
			    {
			        ServerDataBuff.txHead = 0;
			    }
				if( ServerDataBuff.txHead != ServerDataBuff.txTail )  // ??
			    {    		        
			        APP_DEBUG("<--- txHead=%d,txTail=%d --->\r\n",ServerDataBuff.txHead, ServerDataBuff.txTail );
				    if( ServerDataBuff.txHead > ServerDataBuff.txTail )
				    {
				        m_remain_len = HAL_TCP_TX_MAX - ServerDataBuff.txHead;
					  
				    }
					else
					{
					    m_remain_len = ServerDataBuff.txTail - ServerDataBuff.txHead;
					}

					if( ServerDataBuff.txTail == HAL_TCP_TX_MAX )
			        {
			            ServerDataBuff.txTail = 0;
			        }
					
	                m_tcp_state = STATE_SOC_SENDING;
                
	                SocketSendData();
			    }
                break;
            }
            case STATE_SOC_ACK:
            {   				
				#if 1
                u64 ackedNumCurr;
                ret = Ql_SOC_GetAckNumber(m_socketid, &ackedNumCurr);
                if (ret < 0)
                {
                    checkErr_AckNumber(ret);
                }
                if (m_nSentLen == ackedNumCurr)
                {
                    if (timeout_90S_monitor) //stop timeout monitor
                    {
                        Ql_Timer_Stop(TIMEOUT_90S_TIMER_ID);
                        timeout_90S_monitor = FALSE;
                    }
                    
                    APP_DEBUG("<-- ACK Number:%llu/%llu. Server has received all data. -->\r\n\r\n", ackedNumCurr, m_nSentLen);

					
					AfterServerReceiveAll();					
				
					//  APP_DEBUG("\r\n<--FLAG.ISupgrade=%d-->\r\n" ,FLAG.ISupgrade);
                    ////Ql_memset(m_send_buf,0,SEND_BUFFER_LEN);
                    m_tcp_state = STATE_SOC_SEND;
                }
                else
                {
                    if (!timeout_90S_monitor)//start timeout monitor
                    {
                        Ql_Timer_Start(TIMEOUT_90S_TIMER_ID, TIMEOUT_90S_PERIOD, FALSE);
                        timeout_90S_monitor = TRUE;
                    }
                    
                    APP_DEBUG("<-- ACK Number:%llu/%llu from socket[%d] -->\r\n", ackedNumCurr, m_nSentLen, m_socketid);
                }
				#else
				{
				   static u64 SentLenTemp  = 0;
				   if( m_nSentLen != SentLenTemp )
				   {
				       APP_DEBUG("<-- SentTotalLen: %llu from socket[%d] -->\r\n", m_nSentLen, m_socketid);
					   SentLenTemp = m_nSentLen;
					   m_tcp_state = STATE_SOC_SEND;
				   }
				}
				#endif
				
                break;
            }
            case STATE_GPRS_DEACTIVATE:
            {
                APP_DEBUG("<--Deactivate GPRS.-->\r\n");
				
                Ql_GPRS_Deactivate(0);
                break;
            }
            default:
                break;
        	}
        }    
    
}





void Callback_GPRS_Actived(u8 contexId, s32 errCode, void* customParam)
{
    if(errCode == SOC_SUCCESS)
    {
        APP_DEBUG("<--CallBack: active GPRS successfully.-->\r\n");
        m_tcp_state = STATE_GPRS_GET_DNSADDRESS;
    }else
    {
        APP_DEBUG("<--CallBack: active GPRS successfully,errCode=%d-->\r\n",errCode);
        m_tcp_state = STATE_GPRS_ACTIVATE;
    }      
}

void Callback_GetIpByName(u8 contexId, u8 requestId, s32 errCode,  u32 ipAddrCnt, u32* ipAddr)
{
    u8 i=0;
    u8* ipSegment = (u8*)ipAddr;
    
    APP_DEBUG("<-- %s:contexid=%d, requestId=%d,error=%d,num_entry=%d -->\r\n", __func__, contexId, requestId,errCode,ipAddrCnt);
    if (errCode == SOC_SUCCESS)
    {
        APP_DEBUG("<--CallBack: get ip by name successfully.-->\r\n");
        for(i=0;i<ipAddrCnt;i++)
        {
            ipSegment = (u8*)(ipAddr + i);
            APP_DEBUG("<--Entry=%d, ip=%d.%d.%d.%d-->\r\n",i,ipSegment[0],ipSegment[1],ipSegment[2],ipSegment[3]);
        }

        // Fetch the first ip address as the valid IP
        Ql_memcpy(m_ipaddress, ipAddr, 4);
        m_tcp_state = STATE_SOC_REGISTER;
    }
}

void callback_socket_connect(s32 socketId, s32 errCode, void* customParam )
{
    if (errCode == SOC_SUCCESS)
    {
        if (timeout_90S_monitor) //stop timeout monitor
        {
           Ql_Timer_Stop(TIMEOUT_90S_TIMER_ID);
           timeout_90S_monitor = FALSE;
        }

    //    APP_DEBUG("<--Callback: socket connect successfully.-->\r\n");        
	//	LED_STATE=SERVER_LINK;
	//				APP_DEBUG("LED_STATE=%d",SERVER_LINK);
		/***重新连接上时清空接收发送缓存数据***/
		ServerDataBuff.rxTail = 0;
	    ServerDataBuff.rxHead = 0;
	    ServerDataBuff.txTail = 0;
	    ServerDataBuff.txHead = 0;
        m_nSentLen  = 0;
		
		m_tcp_state = STATE_SOC_SEND;
    }
	else
    {
        APP_DEBUG("<--Callback: socket connect failure,(socketId=%d),errCode=%d-->\r\n",socketId,errCode);
        Ql_SOC_Close(socketId);
        m_tcp_state = STATE_SOC_CREATE;
    }
}

void callback_socket_close(s32 socketId, s32 errCode, void* customParam )
{
    m_nSentLen  = 0;
    
    if (errCode == SOC_SUCCESS)
    {
        APP_DEBUG("<--CallBack: close socket successfully.-->\r\n"); 
    }else if(errCode == SOC_BEARER_FAIL)
    {   
        FLAG.linkfFalut[3]=25+errCode;
        m_tcp_state = STATE_GPRS_DEACTIVATE;
        APP_DEBUG("<--CallBack: close socket failure,(socketId=%d,error_cause=%d)-->\r\n",socketId,errCode); 
	}
	else if(errCode == SOC_CONNRESET)
	{
		FLAG.linkfFalut[3]=1;
		APP_DEBUG("\r\n<---web kick---->\r\n");
		m_tcp_state = STATE_GPRS_GET_DNSADDRESS;
		APP_DEBUG("<--CallBack: close socket failure,(socketId=%d,error_cause=%d)...-->\r\n",socketId,errCode); 
	}
	else 
    {
        FLAG.linkfFalut[3]=25+errCode;
        m_tcp_state = STATE_GPRS_GET_DNSADDRESS;
        APP_DEBUG("<--CallBack: close socket failure,(socketId=%d,error_cause=%d)...-->\r\n",socketId,errCode); 
    }
}

void callback_socket_accept(s32 listenSocketId, s32 errCode, void* customParam )
{  
}

void callback_socket_read(s32 socketId, s32 errCode, void* customParam )
{
    s32 ret;
    if(errCode)
    {
        APP_DEBUG("<--CallBack: socket read failure,(sock=%d,error=%d)-->\r\n",socketId,errCode);
        APP_DEBUG("<-- Close socket.-->\r\n");
        Ql_SOC_Close(socketId);
        m_socketid = -1;
        if(errCode == SOC_BEARER_FAIL)  
        {
            m_tcp_state = STATE_GPRS_DEACTIVATE;
        }
        else
        {
            m_tcp_state = STATE_GPRS_GET_DNSADDRESS;
        }  
        return;
    }
 
  // if( NULL == Ql_strstr( (char*)m_SrvADDR, HOST_NAME ) ) //IP address login.
      if(!get_bitFor(handleData.bitFlag, BIT_UPDATA_IP))
      {  
           
		LED_STATE=SERVER_LINK;
		APP_DEBUG("LED_STATE=%d\r\n",SERVER_LINK);
		APP_DEBUG("THE LED IS WORKING STATE\r\n");
	    do 
		{
		    u16 rxLen = 0;
			if( ServerDataBuff.rxTail >= HAL_TCP_RX_MAX )
			{
			    ServerDataBuff.rxTail = 0;
			}
	        if( ServerDataBuff.rxTail+ONE_RECV_MAX_LEN >= HAL_TCP_RX_MAX )
	        {
	            
	            rxLen = HAL_TCP_RX_MAX - ServerDataBuff.rxTail;
				ret = Ql_SOC_Recv(socketId, ServerDataBuff.rxBuf+ServerDataBuff.rxTail, rxLen );
	        }
			else
			{
			    rxLen = ONE_RECV_MAX_LEN;
			    ret = Ql_SOC_Recv(socketId, ServerDataBuff.rxBuf+ServerDataBuff.rxTail, rxLen );
			}

			if( 0 != ret )
			{
				APP_DEBUG("<<<-- Once receive done,ret=%d--rxHead=%d--rxTail=%d-->>>\r\n", ret, ServerDataBuff.rxHead, ServerDataBuff.rxTail ); 
			}
	        				
			if((ret < 0) && (ret != -2))
            {
	            APP_DEBUG("<-- Receive data failure,ret=%d.-->\r\n",ret);
	            APP_DEBUG("<-- Close socket.-->\r\n");
	            Ql_SOC_Close(socketId); //you can close this socket  
	            m_socketid = -1;
	            m_tcp_state = STATE_SOC_CREATE;
	            break;
            }
		    ////else if(ret == -2)
		    ////{
		        //wait next CallBack_socket_read
		        ////break;
		    ////}
	        else if( ret < rxLen || ret == -2 )
	        {  
				ServerDataBuff.rxTail += ret;
			    ServerDataHandle();  //////这个函数不应该放这里，应该放在定时器里?????????????
				break;
	        }
			else if(ret == rxLen )
	        {
	            ServerDataBuff.rxTail += ret;
	        }
		}while(1);
		
	}
    else  //Domain name login. 
	{
		do
    	{
			ret = Ql_SOC_Recv(socketId, UpdataIPbuff, sizeof(UpdataIPbuff) );
			if((ret < 0) && (ret != -2))
            {
	            APP_DEBUG("<-- Receive data failure,ret=%d.-->\r\n",ret);
	            APP_DEBUG("<-- Close socket.-->\r\n");
	            Ql_SOC_Close(socketId); //you can close this socket  
	            m_socketid = -1;
	            m_tcp_state = STATE_SOC_CREATE;
	            break;
            }
		    else if(ret == -2)  
		    {
		        //wait next CallBack_socket_read
		        break;
		    }
	        else
	        {
	            APP_DEBUG("<--Receive data from sock(%d),len(%d):%s\r\n",socketId,ret,UpdataIPbuff);
				UpdataIP_Handle( (char*)UpdataIPbuff, ret, (char*)m_SrvADDR, &m_SrvPort );
				if(get_bitFor(handleData.bitFlag, BIT_UPDATA_IP))handleData.bitFlag = set_bitFor(handleData.bitFlag, BIT_UPDATA_IP,FALSE);

				break;			
	        }		
		}while(1);	
	}
}


void callback_socket_write(s32 socketId, s32 errCode, void* customParam )
{
    s32 ret;

    if(errCode)
    {
        APP_DEBUG("<--CallBack: socket write failure,(sock=%d,error=%d)-->\r\n",socketId,errCode);
        APP_DEBUG("<-- Close socket.-->\r\n");
        ret = Ql_SOC_Close(socketId);
        m_socketid = -1;
        
        if(ret == SOC_BEARER_FAIL)  
        {
            m_tcp_state = STATE_GPRS_DEACTIVATE;
        }
        else
        {
            m_tcp_state = STATE_GPRS_GET_DNSADDRESS;
        }  
        return;
    }  
		
	m_tcp_state = STATE_SOC_SENDING;
	
    SocketSendData();
		
}


void CallBack_GPRS_Deactived(u8 contextId, s32 errCode, void* customParam )
{
    if (errCode == SOC_SUCCESS)
    {
        APP_DEBUG("<--CallBack: deactived GPRS successfully.-->\r\n"); 
        m_tcp_state = STATE_NW_GET_SIMSTATE;
    }
	else
    {
        APP_DEBUG("<--CallBack: deactived GPRS failure,(contexid=%d,error_cause=%d)-->\r\n",contextId,errCode); 

		/***添加以下程序合适否??????***/
		Ql_SOC_Close(m_socketid);
        m_socketid = -1;

        m_tcp_state = STATE_NW_GET_SIMSTATE;
    }
}

static void SocketSendData(void)
{
	do
	{
		ret = Ql_SOC_Send(m_socketid, ServerDataBuff.txBuf+ServerDataBuff.txHead, m_remain_len);
		APP_DEBUG("ServerDataBuff.txHead is %d",ServerDataBuff.txHead);
		APP_DEBUG("the data to be sended is");

            for(int ii=0;ii<m_remain_len;ii++)
            {
				APP_DEBUG(" %0.2x  ",ServerDataBuff.txBuf[ServerDataBuff.txHead+ii]);

			}
		
		APP_DEBUG("\r\n");
		
		APP_DEBUG("<--Send data,socketid=%d,number of bytes sent=%d-->\r\n",m_socketid,ret);   
		
		if(ret == m_remain_len)//send compelete
		{
			ServerDataBuff.txHead += m_remain_len;
			m_nSentLen += ret;
			m_tcp_state = STATE_SOC_ACK;
			break;
		}	
		else if( SOC_WOULDBLOCK == ret ) 
		{
			//waiting CallBack_socket_write, then send data;	 
			break;
		}	
		else if(ret <= 0)
		{
			APP_DEBUG("<--Send data failure,ret=%d.-->\r\n",ret);
			APP_DEBUG("<-- Close socket.-->\r\n");
			Ql_SOC_Close(m_socketid);//error , Ql_SOC_Close
			m_socketid = -1;
 
			if( SOC_BEARER_FAIL == ret )	
			{
				m_tcp_state = STATE_GPRS_DEACTIVATE;
			}
			else
			{
				m_tcp_state = STATE_GPRS_GET_DNSADDRESS;
			}
			break;
		}			
		else if(ret < m_remain_len)//continue send, do not send all data
		{
			m_remain_len -= ret; 
			ServerDataBuff.txHead += ret;
			m_nSentLen += ret;
		}
	}while(1);
}


void GPRS_COPS_Check(void)
{
  
    char strAT[] = "AT+COPS?\0";
	 Ql_RIL_SendATCmd(strAT, Ql_strlen(strAT), GPRS_COPS_Check_Handler, NULL, 0);




}

static s32 GPRS_COPS_Check_Handler(char* line, u32 len, void* userdata)
{
	
	 char *head = Ql_RIL_FindString(line, len, "+COPS:"); //continue wait
	 
	 if(head)
	 {
		 u32 n = 0;
		 u32 state;
		 char state1[20];
		 char state2[20];
		// state2=(char *)userdata; 
		 APP_DEBUG("the line is %s\r\n",line);
		 APP_DEBUG("HEAD IS %s\r\n",head);
		 
		  if(Ql_strstr(head,"CHINA MOBILE"))
		  	{
		  	Ql_memcpy(FLAG.longlat,"1,",2);
            APP_DEBUG( "CHINA MOBILE\r\n");
		  	}
      else if(Ql_strstr(head,"CHINA UNICOM"))
      	{
	  	Ql_memcpy(FLAG.longlat,"2,",2);
            APP_DEBUG("CHINA UINCOM\r\n");
      	}
		 
		else
			{
			Ql_memcpy(FLAG.longlat,"3,",2);
               APP_DEBUG( "UNKNOW CARD\r\n");

		}
	
		 return  RIL_ATRSP_CONTINUE;
	 }
	
	head = Ql_RIL_FindLine(line, len, "OK"); // find <CR><LF>OK<CR><LF>, <CR>OK<CR>，<LF>OK<LF>
	if(head)
	{
		return	RIL_ATRSP_SUCCESS;
	}
	
	 head = Ql_RIL_FindLine(line, len, "ERROR");// find <CR><LF>ERROR<CR><LF>, <CR>ERROR<CR>，<LF>ERROR<LF>
	 if(head)
	 {	
		 return  RIL_ATRSP_FAILED;
	 } 
	
	 head = Ql_RIL_FindString(line, len, "+CME ERROR:");//fail
	 if(head)
	 {
		 return  RIL_ATRSP_FAILED;
	 }
	
	 return RIL_ATRSP_CONTINUE; //continue wait




}


static s32 ATResponse_CREG_LOCATION_Handler(char* line, u32 len, void* userdata)
{
    char *head = Ql_RIL_FindString(line, len, "+CREG:"); //continue wait
    
    if(head)
    {
        u32 n = 0;
		u32 state;
		char state1[20];
		char state2[20];
       // state2=(char *)userdata; 
		APP_DEBUG("the line is %s\r\n",line);
		APP_DEBUG("HEAD IS %s\r\n",head);
        Ql_sscanf(head,"%*[^:]: %d,%d,%s",&n,&state,&FLAG.longlat[2]);
		APP_DEBUG("the location is %s\r\n",FLAG.longlat);
		

        return  RIL_ATRSP_CONTINUE;
    }

   head = Ql_RIL_FindLine(line, len, "OK"); // find <CR><LF>OK<CR><LF>, <CR>OK<CR>，<LF>OK<LF>
   if(head)
   {
       return  RIL_ATRSP_SUCCESS;
   }

    head = Ql_RIL_FindLine(line, len, "ERROR");// find <CR><LF>ERROR<CR><LF>, <CR>ERROR<CR>，<LF>ERROR<LF>
    if(head)
    {  
        return  RIL_ATRSP_FAILED;
    } 

    head = Ql_RIL_FindString(line, len, "+CME ERROR:");//fail
    if(head)
    {
        return  RIL_ATRSP_FAILED;
    }

    return RIL_ATRSP_CONTINUE; //continue wait
}




u32 GPRS_Loction_Check(void)
{
	u32 State;
	u32 data,state;
	u8 buf[64];
	char strAT[] = "AT+CREG?\0";
	 Ql_RIL_SendATCmd(strAT, Ql_strlen(strAT), ATResponse_CREG_LOCATION_Handler, &state, 0);
	// APP_DEBUG("THE LOCATION IS %s\r\n",State);
	return state;
  	
}


s32  SET_CREG(u8 flag)
{
	char strAT[32] = "";

	if(flag)
	{
		
     Ql_memcpy(strAT,"AT+CREG=2\0",Ql_strlen("AT+CREG=2\0"));

	}
	else
	{
		Ql_memcpy(strAT,"AT+CREG=0\0",Ql_strlen("AT+CREG=0\0"));

	}

return Ql_RIL_SendATCmd(strAT, Ql_strlen(strAT), NULL, NULL, 0);



}
/*
void _GPRS_SendData(u8 *pData,u16 len)
{
	u8 ii=0;

  DataEncrypt( pData, len, pData );

  for(ii=0;ii<len;ii++)
  {
   APP_DEBUG("EncryptDataOut[%d]=%x\n",ii,pData[ii]);
  }
  DepositSendingData( pData, len );  

}



void send_data_to_server(u8 com,u8 *pData,u16 len)
{
	
	u8 pTemp[256]={0};
	u16 check = 0;
	u8 l;
	u8 i;
	u8 ii=0;
	u8 err;
	l=(len+11)/8*8+2;		
	((u8*)pTemp)[0]=0xd6;
	((u8*)pTemp)[1]=l;
	((u8*)pTemp)[COM_COM_OFF]=com;
	Ql_memcpy(((u8*)pTemp)+COM_DATA_OFF,pData,len);
	for (i = COM_FRAME_OFF; i < l; i++) {			//
		check += ((u8*)pTemp)[i];
	}
	((u8*)pTemp)[COM_CHECK_OFF] = check;
	((u8*)pTemp)[COM_CHECK_OFF + 1] = check >> 8;

	
	_GPRS_SendData(pTemp,l);

}

void GPSSend(void)
{		
	u8 i=0;
	u8 buf_temp[64]={0};
	Ql_memset(buf_temp,0,sizeof(buf_temp));
//	Ql_memcpy(FLAG.longlat,"1,we34,2222",sizeof(FLAG.longlat));
	if(Ql_strlen(FLAG.longlat))
	{
		((u8*)buf_temp)[0]=0xff;
		((u8*)buf_temp)[1]=0x01;
		#if loction==1
		//((u8*)buf_temp)[2]=0x01;//涔嬪墠gps鐢ㄧ殑1锛岀幇鍦ㄧ敤2		
		((u8*)buf_temp)[2]=0x02;
		#endif           
		((u8*)buf_temp)[3]=Ql_strlen(FLAG.longlat);
	
		Ql_sprintf((u8*)buf_temp+4,"%s", FLAG.longlat);
		send_data_to_server(0x82, (u8*)buf_temp, Ql_strlen(buf_temp));
		Ql_memset(FLAG.longlat,0,sizeof(FLAG.longlat))	;
	}    
}

*/




void AfterServerReceiveAll( void )
{
	 Fota_Pro( handleData.commTaskData.id ); 
	 

}






/*
*/
//void proc_subtask1(s32 TaskId);

void proc_subtask1(s32 TaskId)
{
    bool keepGoing = TRUE;
    ST_MSG subtask1_msg;
    
	APP_DEBUG("<----save task---->\r\n");
    while(keepGoing)
    {    
        Ql_OS_GetMessage(&subtask1_msg);
        switch(subtask1_msg.message)
        {


		//	case MESSAGE_EEPROM_WRITE:
				//memcpy(&handleData,pMessage->data.data8,20);
				//Debug_Printf(7, "writeBill:%d,%d",pMessage->data.data16[2]);
			//	I2CWriteByte(subtask1_msg.param1,(u8*)(pMessage->data.data16+2),pMessage->data.data16[1]);
			//	break;

            case MSG_ID_USER_DATA:
            {
               APP_DEBUG("\r\n<--Sub task 1 recv MSG: SrcId=%d,MsgID=%d Data1=%d, Data2=%d-->\r\n", \
                        subtask1_msg.srcTaskId, \
                        subtask1_msg.message,\
                        subtask1_msg.param1, \
                        subtask1_msg.param2);
                   
                 // 
                break;
            }

            default:
                break;
        }
    }    
}


#endif // __EXAMPLE_TCPCLIENT__

