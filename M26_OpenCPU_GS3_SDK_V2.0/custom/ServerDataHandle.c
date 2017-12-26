

#include "ql_stdlib.h"
#include "ql_type.h"
#include "ql_uart.h"

#include "DES.h"
#include "custom_common.h" 
#include "ServerDataHandle.h"  
#include "my_stdlib.h"
#include "config.h"


static void DataDecrypt( u8 *pDataIn, u16 len, u8 *pDataOut );
static void DataEncrypt( u8 *pDataIn, u16 len, u8 *pDataOut );
static u8 GetServerData( u8 *GetBufOut );
static u8 DepositSendingData(u8 *DPSdataIn, u16 len );
static u8 DataCheck( u8 *pDataIn, u8 len );
extern u8 AnalyseData( u8 *pInAndOut, u8 len, HandleData_sut * pHandleData );
void GPSSend(void);
void send_data_to_server(u8 com,u8 *pData,u16 len);
void _GPRS_SendData(u8 *pData,u16 len);


RxTxCfg_t ServerDataBuff;
const char DES_KEY[8] = { 0xbe, 0x0a, 0x61, 0x37, 0x1a, 0x63, 0xc5, 0x1d };
u8 Server_no_data_send;
void Device_Data_Init(void)
{ 
	des_setkey(DES_KEY);
	ServerDataBuff.rxTail = 0;
	ServerDataBuff.rxHead = 0;
	ServerDataBuff.txTail = 0;
	ServerDataBuff.txHead = 0;
	MY_MEMSET( ServerDataBuff.rxBuf, 0, sizeof(ServerDataBuff.rxBuf) );	
	//MY_DEBUG("<--ServerDataBuff.rxBuf len: %04x.-->\r\n", sizeof(ServerDataBuff.rxBuf) );

	handleData.CoinType.CoinScan = 0;
	handleData.CoinType.CoinScan_Up = 0;
	handleData.CoinType.CoinIn = 0;
	handleData.CoinType.CoinIn_Up = 0;
	handleData.CoinType.PrizePulseIn = 0;
	handleData.CoinType.PrizePulseIn_Up = 0;

	handleData.systemResetFlag = FALSE;
		
	MY_MEMSET( handleData.commTaskData.id, 0, sizeof(handleData.commTaskData.id) );
	MY_MEMSET( handleData.commTaskData.password, 0, sizeof(handleData.commTaskData.password) );
	
	/***TEST***/
	MY_MEMCPY( handleData.commTaskData.id,		LOGIN_ID,		 MY_STRLEN(LOGIN_ID) );
	MY_MEMCPY( handleData.commTaskData.password,	LOGIN_PASSWORD,  MY_STRLEN(LOGIN_PASSWORD) ); 
	APP_DEBUG( "id==%s\r\n", handleData.commTaskData.id );
	APP_DEBUG( "psw==%s\r\n", handleData.commTaskData.password );

}

void hexBufPrintf(char *pTap,u8 *pData, u16 len)//20171226//
{
   MY_DEBUG( "%s:",pTap);  
   
   for(u16 i=0;i<len;i++)
   {
   		MY_DEBUG( "%02X ",pData[i]);  
   }

   MY_DEBUG( ",len=%d bytes\r\n",len);  
}


/***************************************************************************/
static u8 DataCheck( u8 *pDataIn, u8 len )
{                       
    if( len > 5 )
    {  
    	MY_DEBUG( "<----len=%d,zB[0,1,3,2,4,5]=%02X,%02X,%02X%02X,%02X,%02X------------->\r\n",len,*(pDataIn+0),*(pDataIn+1),*(pDataIn+3),*(pDataIn+2),*(pDataIn+4),*(pDataIn+5) );  
        
		hexBufPrintf("<===srvData",pDataIn,len);//20171226//

	}       
			                
	/*检查数据总长度*/
	if( len != pDataIn[COM_LEN_OFF] )  
	{
	  MY_DEBUG( "Data length error.\r\n" );  
	  return FALSE;
	}
   
                                
	/*检查数据头*/
	if ( pDataIn[COM_HEAD_OFF] != COM_DATA_HEAD && pDataIn[COM_HEAD_OFF] != COM_DATA_HEAD2 ) 
	{
	   MY_DEBUG( "Unknown data!\r\n" );     
	   return FALSE;
	}
	           
	/***数据校验***/     
	{   
	    u8 i;
		u16 check0 = 0, check1 = 0;
		check0 = 0;
		for (i = COM_FRAME_OFF; i < pDataIn[COM_LEN_OFF]; i++) 
		{ 
		      check0 += pDataIn[i];
		}

		check1 = pDataIn[ COM_CHECK_OFF] | pDataIn[ COM_CHECK_OFF + 1] << 8;
		if (check0 != check1) 
		{ 
		      MY_DEBUG( "Data checkout fail\r\n" );
		      return FALSE;
		}
	}

    /*检查帧同步*/
    {
        static u8 FrameNum=0; 
		if(pDataIn[0]==0xd5)//D5有同步帧，D6没有
		{

		if ( pDataIn[ COM_CMD_OFF ] != COM_LOGIN ) 
		{             
			if( ++FrameNum != pDataIn[ COM_FRAME_OFF] )  
			{
			  APP_DEBUG( "Data frameNum fail\r\n" );
			  
			  APP_DEBUG("\r\n<----FrameNum=%d,pDataIn[ COM_FRAME_OFF]=%d---->\r\n",FrameNum,pDataIn[ COM_FRAME_OFF])
			  return FALSE;
			}          
		} 
		else
		{
		    FrameNum = pDataIn[ COM_FRAME_OFF];
		}


		}

    }
    return TRUE;
}

/************************************************************************************/
static void DataDecrypt( u8 *pDataIn, u16 len, u8 *pDataOut )
{
   if( len >  COM_LEN_OFF )
   {
      if( len == pDataIn[COM_LEN_OFF] )
      {
        if(       pDataIn[COM_LEN_OFF] >= COM_CHECK_OFF + COM_ENCRYPT_LEN 
             && ( pDataIn[COM_HEAD_OFF] == 0xD5 || pDataIn[COM_HEAD_OFF] == 0xD6 ) 
          )
        {
          u16 i;
          for ( i = 0; i < ( pDataIn[COM_LEN_OFF] - COM_CHECK_OFF ) / COM_ENCRYPT_LEN; i++ ) 
          {
            des_run( (char*) pDataOut  + COM_CHECK_OFF + i * COM_ENCRYPT_LEN, 
                     (char*) pDataIn  + COM_CHECK_OFF + i * COM_ENCRYPT_LEN, 
                     decrypt ); 
          }		   
        }   
      } 
   }
}

/************************************************************************************/
static void DataEncrypt( u8 *pDataIn, u16 len, u8 *pDataOut )
{
      if( len >  COM_LEN_OFF && pDataIn[ COM_LEN_OFF ] > COM_CHECK_OFF  )
      { 
        u16 i;
        for ( i = 0; i < ( pDataIn[ COM_LEN_OFF ] - COM_CHECK_OFF  ) / COM_ENCRYPT_LEN; i++ ) 
        {
          des_run(  (char*) pDataOut  + COM_CHECK_OFF + COM_ENCRYPT_LEN * i, 
                    (char*) pDataIn   + COM_CHECK_OFF + COM_ENCRYPT_LEN * i, 
                    encrypt);
        }		
      }
	  else
	  {
	    MY_DEBUG("<-- Data encrypt failure-->\r\n");
	  }
}

/************************************************************************************/
u8 GetServerData( u8 *GetBufOut )
{
	u16 cnt = 0;
	u16 rxHeadTemp = ServerDataBuff.rxHead;
	while ( ( ServerDataBuff.rxHead != ServerDataBuff.rxTail ) )
	{
		GetBufOut[cnt++] = ServerDataBuff.rxBuf[ ServerDataBuff.rxHead++ ];
		
		if ( ServerDataBuff.rxHead >= HAL_TCP_RX_MAX )
		{
		  ServerDataBuff.rxHead = 0;
		}

		if( COM_DATA_HEAD == GetBufOut[COM_HEAD_OFF] || COM_DATA_HEAD2 == GetBufOut[COM_HEAD_OFF] )
		{ 
			if( cnt > COM_LEN_OFF )
			{   
			    if( GetBufOut[COM_LEN_OFF] <= COM_RSV_MAX_LEN && GetBufOut[COM_LEN_OFF] >= COM_RSV_MIN_LEN )
			    {
					if( cnt == GetBufOut[COM_LEN_OFF] )
					{
						//20171226//MY_DEBUG("<-- Get a whole packet successfully -->\r\n");
						return cnt;
					}
			    }
				else 
				{
				    cnt = 0; 
					rxHeadTemp = ServerDataBuff.rxHead;
					MY_DEBUG("<-- packet length wrong -->\r\n");
				}				
			}			
		}
		else
		{
			cnt = 0;  
			rxHeadTemp = ServerDataBuff.rxHead;
			MY_DEBUG("<-- packet head data wrong -->\r\n");
		}
	}

	if( cnt < COM_RSV_MAX_LEN && cnt > 0 )  //小于最长的一个数据包时。
	{
		ServerDataBuff.rxHead = rxHeadTemp;	//恢复到原来的位置。
		cnt = 0;
		MY_DEBUG("<-- Get a whole packet failure -->\r\n");
	}
	return cnt;

}
/******************************************************************************************/
#define HAL_DEPOSIT_AVAIL(x) \
       ( ( x.txHead > x.txTail) ? \
         ( x.txHead - x.txTail ) : \
         ( HAL_TCP_TX_MAX - x.txTail + x.txHead ) )

static u8 DepositSendingData(u8 *DPSdataIn, u16 len )
{   
    //MY_DEBUG("<--remainLen:%d,txHead:%d--txTail:%d-->\r\n", HAL_DEPOSIT_AVAIL( ServerDataBuff ),ServerDataBuff.txHead, ServerDataBuff.txTail );      
    if ( HAL_DEPOSIT_AVAIL( ServerDataBuff ) < len )
    {
        MY_DEBUG("<--Have not enough space to deposit data,remain len:%d,sending len:%d-->\r\n", HAL_DEPOSIT_AVAIL( ServerDataBuff ),len );
        return 0;
    }	
	
	if( ServerDataBuff.txTail >= HAL_TCP_TX_MAX )
	{
	    ServerDataBuff.txTail = 0;
	}
	if( ServerDataBuff.txTail+len > HAL_TCP_TX_MAX )
	{
	    
		MY_MEMCPY( ServerDataBuff.txBuf+ServerDataBuff.txTail, DPSdataIn, HAL_TCP_TX_MAX - ServerDataBuff.txTail );
		MY_MEMCPY( ServerDataBuff.txBuf, DPSdataIn+(HAL_TCP_TX_MAX - ServerDataBuff.txTail), len -( HAL_TCP_TX_MAX - ServerDataBuff.txTail ) );
		ServerDataBuff.txTail = len -( HAL_TCP_TX_MAX - ServerDataBuff.txTail );
	}
	//else if( ServerDataBuff.txTail+len == HAL_TCP_TX_MAX )
	//{
	    //MY_MEMCPY( ServerDataBuff.txBuf+ServerDataBuff.txTail, DPSdataIn, len );
		//ServerDataBuff.txTail = 0;//注:若此时ServerDataBuff.txHead == 0,则数据缓存为空，不会发送数据，此段程序放到发送处去处理。	
	//}
	else
	{
	    MY_MEMCPY( ServerDataBuff.txBuf+ServerDataBuff.txTail, DPSdataIn, len );
		ServerDataBuff.txTail += len;
	}	
	
	return len;
}



u32 ipaddr_addr(char * addr)
{
	u32 d;
	u32 d0,d1,d2,d3;
	Ql_sscanf(addr,"%d.%d.%d.%d",&d3,&d2,&d1,&d0);
	d = (u32)d3<<24 & 0xff000000;
	d |= (u32)d2<<16 & 0x00ff0000;
	d |= (u32)d1<<8 & 0x0000ff00;
	d |= d0&0x000000ff;
	//Debug_Printf(5, "ipaddr_addr:%d\r\n",d);
	return d;
}

  
  // /*  char *p1 = NULL, *p2 = NULL;
  //	p1 = MY_STRCHR( (const char*)pIPdataIn , '|');
  //	if( NULL == p1 )
  //	{
  //	MY_DEBUG("<<<--Received IP address error,1-->>>\r\n");
  //	return FALSE;
  //	}
  //	p2 = MY_STRCHR( (const char*)pIPdataIn, ':');
  //	if( NULL == p2 )
  //	{
  //	MY_DEBUG("<<<--Received IP address error,2-->>>\r\n");
  //	return FALSE;
  //	} 
  //	if( len <= p2 - pIPdataIn + 1 )
  //	{
  //	MY_DEBUG("<<<--Received IP address lenght error-->>>\r\n");
  //	return FALSE;
  //	} */
  //	/***解析IP***/
  //	/*{
  //	if( (p2-p1-1 < 7) || (p2-p1-1 > 15) ) ////String IP length range in 7~15 byte.
  //	{
  //	   MY_DEBUG("<<<--Fail to Updata IP address-->>>\r\n");
  //	   return FALSE;
  //	}
  //	else
  //	{
  //	   MY_STRNCPY( pIPaddrOut, (const char*)p1+1, p2-p1-1 );
  //	   pIPaddrOut[p2-p1-1] = '\0';			   
  //	   MY_DEBUG("<<<--Updata IP address successfully:%s-->>>\r\n", pIPaddrOut );
  //	}		
  //	}*/
  
  //	/***解析端口***/
  //	  
  //   /*	u32 IPport = Ql_atoi( (const char*)p2+1 );
  //	if( 0 == IPport )
  //	{
  //	   MY_DEBUG("<<<-- Fail to Updata IP port -->>>\r\n");
  //	   return FALSE;
  //	}
  //	else
  //	{
  //	   *pIPportOut = IPport;
  //	   MY_DEBUG("<<<--Updata Ip port successfully,IPport=%d-->>>\r\n", *pIPportOut );	   
  //	}
  //   
  
  
  // return TRUE;*/



/****************************************************************************************/
extern void device_ParaSave(Device_sut * pDevice);//20171226//

u8 UpdataIP_Handle( char *pIPdataIn, s32 len,  char *pIPaddrOut, u32 *pIPportOut )
{


	 	s8 r=0;
	int i=0,j=0;
	u16 off=0,all=0;
	u32 data=0;
	char buf0[16];
	u8 updata;
	
    if( len <= 0 )
    {
       return FALSE;
    }

	if(len==0){		
		
	}else{
		pIPdataIn[len]=0;
		APP_DEBUG("\r\npIPdataIn=%s\r\n",pIPdataIn);
		while (pIPdataIn[++i] != '|' && pIPdataIn[i] != '\0');
		if(pIPdataIn[i]=='|'){
			Ql_strncpy(buf0, (const char*)pIPdataIn + j, i - j);
			buf0[i - j] = '\0';
			j = i + 1;
			off = Ql_atoi(buf0);
			if(off>5)off=5;
			APP_DEBUG("off=%d\r\n",off);
			if ((off != device.commTaskData.serverOff)||(off==0)){
				device.commTaskData.serverOff = off;
				//send_Message(TASK_IO, MESSAGE_SERVER, SERVER_OFF, (u8 *)&off, 2);
			}
			
			while (1) {	//解析ip和端口
				//解析ip
				while (pIPdataIn[++i] != ':' && pIPdataIn[i] != '\0');
				if(pIPdataIn[i]==':'){
					Ql_strncpy(buf0, (const char*)pIPdataIn + j, i - j);
					buf0[i - j] = '\0';
					j = i + 1;
					data = ipaddr_addr(buf0);
					APP_DEBUG("ip=%d\r\n",data);
					if (data != device.commTaskData.ip[all]) {
						if (!updata)
							updata = TRUE;
						device.commTaskData.ip[all] = data;
					}
					//解析端口
					while (pIPdataIn[++i] != ',' && i < len && pIPdataIn[i] != '\0');
					Ql_strncpy(buf0, (const char*)pIPdataIn + j, i - j);
					buf0[i - j] = '\0';
					j = i + 1;
					data = Ql_atoi(buf0);
					APP_DEBUG("port=%d\r\n",data);
					if (data != device.commTaskData.port[all]) {
						if (!updata)
							updata = TRUE;
						device.commTaskData.port[all] = data;
					}

					all++;
				}
				if (i >= len || pIPdataIn[i] == '\0' || all >= 5) {
					break;
				}

			}
			
			APP_DEBUG("all=%d\r\n",all);
			if (device.commTaskData.serverAll != all) {
				device.commTaskData.serverAll = all;
				//send_Message(TASK_IO, MESSAGE_SERVER, SERVER_ALL, (u8 *)&all, 2);
				//device_ParaSave(pDevice,DEVICE_SERVER_ALL);
			}

			if (updata) {
				//device_ParaSave(pDevice,DEVICE_IP);
				//device_ParaSave(pDevice,DEVICE_PORT);
				//send_Message(TASK_IO, MESSAGE_SERVER, SERVER_IP, (u8 *)pHandleData->commTaskData.ip, 20);
				//send_Message(TASK_IO, MESSAGE_SERVER, SERVER_PORT, (u8 *)pHandleData->commTaskData.port, 10);
			}

			device_ParaSave(&device);
		}

		
		}

	return r;
}


/****************************************************************************************/
void ServerDataHandle(void)
{   
    u8 GetLen = 0;       
    u8 GetBuf[ COM_ANSWER_MAX_LEN ];
//if(FLAG.ISupgrade>=2) return;
	MY_MEMSET( GetBuf, 0, sizeof(GetBuf) );	
    GetLen = GetServerData( GetBuf );  
	if( !GetLen )
	{
	    Server_no_data_send=1;
	    MY_DEBUG("<<<--Get nothing from Server-->>>\r\n" );
		return;
	}
	else
		Server_no_data_send=0;
	DataDecrypt( GetBuf, GetLen, GetBuf );
  
    if( TRUE != DataCheck( GetBuf, GetLen ) ) 
    {
        return;
    }


       handleData.linkTime=handleData.reConntInterval;

	{
		u8 retANLSlen = 0;
		retANLSlen = AnalyseData( GetBuf, GetLen, &handleData );
	    if( retANLSlen <= 0 )
	    {
	       return;
	    }   
		hexBufPrintf("===>ackSrvData", GetBuf, retANLSlen);//20171226//
		
		DataEncrypt( GetBuf, retANLSlen, GetBuf );
		DepositSendingData( GetBuf, retANLSlen );       
	}
}




void _GPRS_SendData(u8 *pData,u16 len)
{
  hexBufPrintf("===>send2Srv:", pData, len);//20171226//

  DataEncrypt( pData, len, pData );
  DepositSendingData( pData, len );  
}



void send_data_to_server(u8 com,u8 *pData,u16 len)
{
	
	u8 pTemp[256]={0};
	u16 check = 0;
	u8 l;
	u8 i;
	//20171226//u8 ii=0;
	//20171226//u8 err;
	l=(len+11)/8*8+2;		
	pTemp[0]=0xd6;
	pTemp[1]=l;
	pTemp[COM_COM_OFF]=com;
	Ql_memcpy(pTemp+COM_DATA_OFF,pData,len);
	for (i = COM_FRAME_OFF; i < l; i++) {			//
		check += pTemp[i];
	}
	pTemp[COM_CHECK_OFF] = check;
	pTemp[COM_CHECK_OFF + 1] = check >> 8;

	
	_GPRS_SendData(pTemp,l);

}


void send_data_to_server1(u8 com,u8 *pData,u16 len,u8 frame){
	u8 pTemp[256]={0};
	u16 check = 0;
	u8 l=0;
	u8 i=0;
	//20171226//u8 err=0;
	l=(len+11)/8*8+2;
		

	pTemp[0]=0xd6;
	pTemp[1]=l;
	pTemp[COM_FRAME_OFF]=frame;
	pTemp[COM_COM_OFF]=com;
	Ql_memcpy(pTemp+COM_DATA_OFF,pData,len);
	for (i = COM_FRAME_OFF; i < l; i++) {			
		check += pTemp[i];
	}
	pTemp[COM_CHECK_OFF] = check;
	pTemp[COM_CHECK_OFF + 1] = check >> 8;
	_GPRS_SendData(pTemp,l);



}





