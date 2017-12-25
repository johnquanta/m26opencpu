

#include "ql_stdlib.h"
#include "ql_type.h"
#include "ql_uart.h"
#include "custom_common.h"
#include "ServerDataHandle.h"
#include "my_stdlib.h"

#include "fota_by_http.h"

#include "config.h"
#include "coin_prize.h"




#define TEST_DEBUG_HANDLE 1
 u8 restart;

 u8  CmdType;
 u16 CmdData;

 Device_sut device;
  S_FLAG FLAG;

  HandleData_sut handleData; 

//  u8 LINKED=0;


u32 get_bitFor( u32 data, u8 index )
{
	return( (data >> index) & 1);
}


u32 set_bitFor( u32 data, u8 index, u8 d )
{
	if ( d )
	{
		data |= 1 << index;
	} else {
		data &= ~(1 << index);
	}
	return(data);
}


u8 AnalyseData( u8 *p, u8 length, HandleData_sut * pHandleData )
{
	s8	r = 0;
	int	i;
	u16	check0	= 0, check1 = 0;
	u8	len	= 0;
	u16	coin	= 0;
	/* u8 *p; */
	u16	io=0;
	u8	s=0;
	u8	pTemp[128]={0};


	u8 err;
	//if ( (COM_DATA_HEAD != p[0]) || (0 >= length) )
	//	return(0);

	if ( p[0] == 0xD5 )
	{

		MY_DEBUG("\r\np[COM_COM_OFF]=%x\r\n",p[COM_COM_OFF]);

		if ( p[COM_COM_OFF] != COM_LOGIN )
		{
			if ( ++pHandleData->frame != p[COM_FRAME_OFF] ) /* 帧不同步 */
			{
				MY_DEBUG( "frame fail\r\n" );
			}
		}else{
			pHandleData->frame = p[COM_FRAME_OFF];
		}
		MY_DEBUG( "frame=%d\r\n", p[COM_FRAME_OFF] );

		/*
		 * pHandleData->transferTime = OSTimeGet();
		 * pHandleData->updataIPTime = pHandleData->transferTime;
		 */
		if ( p[COM_COM_OFF] == COM_SET )
		{
			pHandleData->loginTime		= pHandleData->transferTime;
			pHandleData->conntInterval	= 0;

			FLAG.deviceSendTime = p[COM_DATA_OFF + 2 + 5];
			if ( FLAG.deviceSendTime == 0 )
			{
				FLAG.deviceSendTime = 60;
			}
			MY_DEBUG( "DS=%d\r\n", FLAG.deviceSendTime );
			LED_STATE=LED_STATE_WORK;


			pHandleData->bitFlag = set_bitFor(pHandleData->bitFlag, BIT_LINK, TRUE);
			FLAG.Linked = 1;
		//	LINKED=1;
          MY_DEBUG("connect the server success\r\n");
			
		}else  {
			FLAG.deviceSendBackup = FLAG.deviceSendTime ;
			MY_DEBUG("收到FLAG.deviceSendTime= %d\r\n",FLAG.deviceSendTime);
		}


		if ( p[COM_COM_OFF] == COM_LOGIN )
		{
			u32 Versoin;
			len	= COM_LOGIN_LEN;
			Versoin = COMM_VERSOIN;
			MY_MEMCPY( p + COM_DATA_OFF, &Versoin, 4 );
			MY_MEMCPY( &p[10], &device.commTaskData.id, 8 );                   /*  */
			MY_MEMCPY( &p[18], &device.commTaskData.password, 8 );             /*  */
			MY_MEMCPY( &p[26], &device.commTaskData.bill.AllCoin, 4 );        /*  */

			i = DEVICE_VERSOIN;
			MY_MEMCPY( p + 30, &i, 4 );
		} else if ( p[COM_COM_OFF] == COM_COIN )                                        /*  */
		{
			coin	= p[COM_DATA_OFF] | p[COM_DATA_OFF + 1] << 8;                   /*  */
			len	= COM_COIN_LEN;                                                 /*  */
			MY_MEMCPY( p + COM_DATA_OFF, &device.commTaskData.bill.AllCoin, 4 );    /*  */

			if ( coin > device.commTaskData.ioPara.plusePara[MAX_UP_COIN] )
			{
				FLAG.FalutCoinFlag	|= 0x04;
				FLAG.FalutCoin[2]	= coin;
				coin			= 0;                                    /* ??1000?? */
			}


		}else if ( p[COM_COM_OFF] == COM_SET )
		{
			len = COM_SET_LEN;
			if ( pHandleData->commTaskData.ioPara.res )
			{
				pHandleData->commTaskData.ioPara.res = 0;

				/*	send_Message(TASK_IO,MESSAGE_IO_PARA_RES,0,(u8*)&pHandleData->commTaskData.ioPara.res,2); */
			}


			io = p[COM_DATA_OFF + 2] | p[COM_DATA_OFF + 3] << 8;
			if ( io < 15 )
				io = 15;
			pHandleData->reConntInterval = io;

			handleData.linkTime=FLAG.linkTimeBack=io;

			MY_DEBUG( "Re=%d\r\n", pHandleData->reConntInterval );


			MY_MEMSET( p + COM_DATA_OFF, 0, 4 );

			if ( get_bitFor( pHandleData->bitFlag, BIT_CLOSE_POWER ) )
			{
				pHandleData->bitFlag	= set_bitFor( pHandleData->bitFlag, BIT_CLOSE_POWER, FALSE );
				p[COM_DATA_OFF]		|= 0X02;
			}else  {
			}

			FLAG.deviceSendBackup	= 5;
			p[COM_DATA_OFF]		|= 0X04; /* 重发标志 */

			if ( FLAG.FirstPower == 0 )
			{
				FLAG.WhichToSend = DEVICE_CCID;
			}else  {
			}
		} else if ( p[COM_COM_OFF] == COM_STATE )
		{
			len = COM_STATE_LEN;
			MY_MEMSET( p + COM_DATA_OFF, 0, 4 );
			MY_MEMCPY( p + COM_DATA_OFF, &pHandleData->rssi, 1 );
		} else if ( p[COM_COM_OFF] == COM_COIN_IN )     /* ???? */
		{
			len = COM_COIN_IN_LEN;
			MY_MEMSET( p + COM_DATA_OFF, 0, len - COM_DATA_OFF );
			MY_MEMCPY( p + COM_DATA_OFF, &device.commTaskData.bill.AllCoinIn, 2 );
			MY_MEMCPY( p + COM_DATA_OFF + 2, &device.commTaskData.bill.AllExtendIn, 2 );
			MY_MEMCPY( p + COM_DATA_OFF + 4, &device.commTaskData.bill.AllExtendVirtual, 2 );
		} else if ( p[COM_COM_OFF] == COM_SET_ID )      /* ???? */
		{
			len = COM_SET_ID_LEN;
			MY_MEMCPY( pHandleData->commTaskData.id, p + COM_DATA_OFF, 8 );
			MY_MEMCPY( pHandleData->commTaskData.password, p + COM_DATA_OFF + 8, 8 );
			MY_MEMSET( p + COM_DATA_OFF, 0, len - COM_DATA_OFF );

			/* send_Message(TASK_IO,MESSAGE_ID_SET,0,(u8*)&pHandleData->commTaskData.id,20); */

			/* if(FLASH_COMPLETE==device_ParaSave(pDevice, DEVICE_ID)) */
			p[COM_DATA_OFF] = 1;
		} else if ( p[COM_COM_OFF] == COM_GET_HARD )    /* ???? */
		{
			len = COM_GET_HARD_LEN;
					if ( p[COM_DATA_OFF] == 1 )             /* 硬件CHIP_ID. */
		{
			u32 id = 0;                             /* system_get_chip_id(); */
			MY_MEMSET( p + COM_DATA_OFF, 0, len - COM_DATA_OFF );
			p[COM_DATA_OFF]		= 1;
			p[COM_DATA_OFF + 1]	= 4;
			MY_MEMCPY( p + COM_DATA_OFF + 2, &id, p[COM_DATA_OFF + 1] );
		}else if ( p[COM_DATA_OFF] == 0 )       /* 網卡地址MAC. */
		{
			MY_MEMSET( p + COM_DATA_OFF, 0, len - COM_DATA_OFF );
			p[COM_DATA_OFF]		= 0;
			p[COM_DATA_OFF + 1]	= 6;
		}else if ( p[COM_DATA_OFF] == 2 )       /* SIM卡IMSI號. */
		{
			MY_MEMSET( p + COM_DATA_OFF, 0, len - COM_DATA_OFF );
			p[COM_DATA_OFF]		= 2;
			p[COM_DATA_OFF + 1]	= 8;
			MY_MEMCPY(p + COM_DATA_OFF + 2, &pHandleData->ismi0, 4);
			MY_MEMCPY(p + COM_DATA_OFF + 6, &pHandleData->ismi1, 4);
			//MY_MEMCPY( p + COM_DATA_OFF + 2, pHandleData->ismi, p[COM_DATA_OFF + 1] );
			//MY_MEMCPY( p + COM_DATA_OFF + 2, , p[COM_DATA_OFF + 1] );
		}else if ( p[COM_DATA_OFF] == 3 )       /* CCID号 */
		{
			MY_MEMSET( p + COM_DATA_OFF, 0, len - COM_DATA_OFF );
			p[COM_DATA_OFF]		= 3;
			p[COM_DATA_OFF + 1]	= 10;
			MY_MEMCPY( p + COM_DATA_OFF + 2, pHandleData->iccid, p[COM_DATA_OFF + 1] );
		}else  {
			MY_MEMSET( p + COM_DATA_OFF, 0, len - COM_DATA_OFF );
			p[COM_DATA_OFF]		= 0xff;
			p[COM_DATA_OFF + 1]	= 0;
		}
		} else if ( p[COM_COM_OFF] == COM_CMD )                                                 /* ???? */
		{
			len = COM_CMD_LEN;

			if ( MY_STRCMP( "_restart", (const char *) p + COM_DATA_OFF ) == 0 )            /* ???? */
			{
				MY_MEMSET( p + COM_DATA_OFF, 0, len - COM_DATA_OFF );
				MY_MEMCPY( p + COM_DATA_OFF, "_ok", MY_STRLEN( "_ok" ) );
				restart=1;
				pHandleData->bitFlag = set_bitFor( pHandleData->bitFlag, BIT_RESTART, TRUE );
			} else if ( MY_STRCMP( "_restore", (const char *) p + COM_DATA_OFF ) == 0 )     /* ???? */
			{
				MY_MEMSET( p + COM_DATA_OFF, 0, len - COM_DATA_OFF );
				MY_MEMCPY( p + COM_DATA_OFF, "unknown", MY_STRLEN( "unknown" ) );
				/* pDevice->bitFlag = set_bitFor(pDevice->bitFlag, BIT_UPDATA_FINISH, TRUE); */
			}else if ( MY_STRCMP( "_bill", (const char *) p + COM_DATA_OFF ) == 0 )         /* ???? */
			{
				MY_MEMSET( p + COM_DATA_OFF, 0, len - COM_DATA_OFF );
				
				MY_SPRINTF( (char *) pTemp, "%d,%d", device.commTaskData.bill.AllCoin, device.commTaskData.bill.AllCoinIn );
				MY_MEMSET( p + COM_DATA_OFF, 0, len - COM_DATA_OFF );
				MY_MEMCPY( p + COM_DATA_OFF, pTemp, MY_STRLEN( (char *) pTemp ) );
				
			}   else if ( MY_STRCMP( "_no_nc", (const char *) p + COM_DATA_OFF ) == 0 ) /* ???? */
			{
				MY_MEMSET( p + COM_DATA_OFF, 0, len - COM_DATA_OFF );

				/* pTemp = OSMemGet(MemBig,&err); */
				if ( FLAG.CoinStat == 1 )
				{
					MY_SPRINTF( (char *) pTemp, "no,%d", FLAG.CoinPlusCount );
				}   else{
					MY_SPRINTF( (char *) pTemp, "nc,%d", FLAG.CoinPlusCount );
				}

				MY_MEMCPY( p + COM_DATA_OFF, pTemp, MY_STRLEN( (char *) pTemp ) );
				/*			OSMemPut(MemBig,pTemp); */
			}else if ( p[COM_DATA_OFF] == '*' )
			{
				MY_DEBUG( (const char *) p + COM_DATA_OFF );
				MY_DEBUG( "\r\n" );
				/*
				 * s = MY_ATOI((const char*)p + COM_DATA_OFF + 1);
				 * io = MY_ATOI((const char*)p + COM_DATA_OFF + 3);
				 */

				//u8	CmdType;
			 //   u16	CmdData;
				Ql_sscanf( (const char *) p + COM_DATA_OFF + 1, "%d,%d",  &CmdType,  &CmdData);
				 MY_DEBUG("s,io=%d,%d\r\n", CmdType,CmdData); 
				s=CmdType;
				s = MY_ATOI((const char*)p + COM_DATA_OFF + 1);
				MY_DEBUG("s is %d\r\n",s);
				io=CmdData;

				if ( s < IOPARA_NUM )
				{
					io /= HEART_INTERVAL;
					if ( pHandleData->commTaskData.ioPara.plusePara[s] != io )
					{
						pHandleData->commTaskData.ioPara.plusePara[s] = io;
						device.commTaskData.ioPara.plusePara[s]= io;
						
						device_ParaSave(&device);
						/*	send_Message(TASK_IO,MESSAGE_IO_PARA_PLUSE,s,(u8*)&pHandleData->commTaskData.ioPara.plusePara[s],2); */
					}
					io *= HEART_INTERVAL;
				}else if ( s >= REMOTE_SET_OFF && s < REMOTE_SET_OFF + 16 )
				{
					MY_DEBUG("s3,io3=%d,%d\r\n", s,io); 

				
					if ( get_bitFor( pHandleData->commTaskData.ioPara.set, s - REMOTE_SET_OFF ) && io == 0 )
					{
					device.commTaskData.ioPara.set=	pHandleData->commTaskData.ioPara.set = set_bitFor( pHandleData->commTaskData.ioPara.set, s - REMOTE_SET_OFF, FALSE );
					//MY_DEBUG("s1,io1=%d,%d\r\n", s,io); 

						device_ParaSave(&device);

						/*	send_Message(TASK_IO,MESSAGE_IO_PARA_SET,s-REMOTE_SET_OFF,(u8*)&pHandleData->commTaskData.ioPara.set,2); */
					}else if ( get_bitFor( pHandleData->commTaskData.ioPara.set, s - REMOTE_SET_OFF ) == 0 && io )
					{
						device.commTaskData.ioPara.set=pHandleData->commTaskData.ioPara.set = set_bitFor( pHandleData->commTaskData.ioPara.set, s - REMOTE_SET_OFF, TRUE );
						device_ParaSave(&device);
						//MY_DEBUG("s2,io2=%d,%d\r\n", s,io); 

						/*	send_Message(TASK_IO,MESSAGE_IO_PARA_SET,s-REMOTE_SET_OFF,(u8*)&pHandleData->commTaskData.ioPara.set,2); */
					}
				}


				/* pTemp = OSMemGet(MemBig,&err); */
				MY_SPRINTF( (char *) pTemp, "%d,%d", s, io );
				MY_MEMSET( p + COM_DATA_OFF, 0, len - COM_DATA_OFF );
				MY_MEMCPY( p + COM_DATA_OFF, pTemp, MY_STRLEN( (char *) pTemp ) );
				/* OSMemPut(MemBig,pTemp); */
			} else if ( p[COM_DATA_OFF] == '&' )
			{
				MY_DEBUG( (const char *) p + COM_DATA_OFF );
				MY_DEBUG( "\r\n" );
				s = MY_ATOI( (const char *) p + COM_DATA_OFF + 1 );
				/* Debug_Printf(5,"s=%d\r\n", s); */

				if ( s < IOPARA_NUM )
				{
					io	= pHandleData->commTaskData.ioPara.plusePara[s];
					io	*= HEART_INTERVAL;
				}else if ( s >= REMOTE_SET_OFF && s < REMOTE_SET_OFF + 16 )
				{
					if ( get_bitFor( pHandleData->commTaskData.ioPara.set, s - REMOTE_SET_OFF ) )
					{
						io = 1;
					}else{
						io = 0;
					}
				}else if ( s >= REMOTE_SET_OFF + 20 && s < REMOTE_SET_OFF + 24 )
				{
					/* io = pHandleData->inPluseP[s-REMOTE_SET_OFF-20]; */
					io *= HEART_INTERVAL;
				}


				/* pTemp = OSMemGet(MemBig,&err); */
				MY_SPRINTF( (char *) pTemp, "%d,%d", s, io );
				MY_MEMSET( p + COM_DATA_OFF, 0, len - COM_DATA_OFF );
				MY_MEMCPY( p + COM_DATA_OFF, pTemp, MY_STRLEN( (char *) pTemp ) );
				/* OSMemPut(MemBig,pTemp); */
			}  else{
				MY_MEMSET( p + COM_DATA_OFF, 0, len - COM_DATA_OFF );
				MY_MEMCPY( p + COM_DATA_OFF, "unknown", MY_STRLEN( "unknown" ) );
			}
		} else if ( p[COM_COM_OFF] == COM_FOTA ) /* ???? */
		{ 
			len = COM_FOTA_LEN;

			Fota_DataHandle( p );

			MY_MEMSET( p + COM_DATA_OFF, 0, len - COM_DATA_OFF );

			p[COM_DATA_OFF] = 1;
			
		} else if ( p[COM_COM_OFF] == COM_LOGIN_RES ) /* ???? */
		{
			len = COM_LOGIN_RES_LEN;

			if ( (u8) pHandleData->commTaskData.ioPara.res != p[COM_DATA_OFF] )
			{
				pHandleData->commTaskData.ioPara.res = p[COM_DATA_OFF];
				/* device_ParaSave(pDevice, DEVICE_RES); */

				/* send_Message(TASK_IO,MESSAGE_IO_PARA_RES,0,(u8*)&pHandleData->commTaskData.ioPara.res,2); */
			}


			if ( pHandleData->commTaskData.ioPara.res == 1 || pHandleData->commTaskData.ioPara.res == 2 )
			{
				pHandleData->bitFlag = set_bitFor( pHandleData->bitFlag, BIT_BAN_LINK, 1 );

			    LED_STATE=LED_STATE_STOP_LINK;

			}
				
			MY_MEMSET( p + COM_DATA_OFF, 0, len - COM_DATA_OFF );
		}else if ( p[COM_COM_OFF] == COM_RECEIVE_USART )        /*下發BIN 串口命令 */
		{
			if ( p[COM_DATA_OFF + 1] == 0x02 )
			{
			}else if ( p[COM_DATA_OFF + 1] == 0x14 )        /* 二维码机台设置 */
			{
				/* com_user_send(COM485_GETMACHINESETTING+0x80,&p[COM_COM_OFF+3],7); */
			}else if ( p[COM_DATA_OFF + 1] == 0xb1 )        /* apn设置 */
			{
			}

			/* send_Message(TASK_IO, MESSAGE_USART_COM, p[COM_DATA_OFF]+1, p+COM_DATA_OFF, p[COM_DATA_OFF]+1); */
			len = COM_RECEIVE_USART_LEN;
			MY_MEMSET( p + COM_DATA_OFF, 0, len - COM_DATA_OFF );
			p[COM_DATA_OFF] = 1;
		}else { /* ???? */
			len = COM_LEN_MIN;
			MY_MEMSET( p + COM_DATA_OFF, 0, len - COM_DATA_OFF );
		}

		/* ?? ?? */
		u16	checkOut = 0;
		u8	ic;
		p[COM_CMD_OFF]++; /* //应答命令+1； */
		p[COM_LEN_OFF] = len;

		for (ic = COM_FRAME_OFF; ic < len; ic++ )
		{
			checkOut += p[ic];
		}
		/* MY_DEBUG( "DaFLAGFLAGta check0==%02X%02X\r\n",(u8)(checkOut >> 8), (u8)checkOut ); */
		p[COM_CHECK_OFF]	= (u8) checkOut;
		p[COM_CHECK_OFF + 1]	= (u8) (checkOut >> 8);


		if ( coin )
		{
	
			pHandleData->CoinType.CoinScan += coin;
		    MY_DEBUG( "CoinScan=%d\r\n", pHandleData->CoinType.CoinScan);
		}
	}else{
#if RESEND == 1
		if ( p[0] == 0xD6 )
		{
			/*
			 * pHandleData->transferTime = OSTimeGet();
			 * pHandleData->updataIPTime = pHandleData->transferTime;
			 * Debug_Printf(5,"应答\r\n",buf);
			 */

			if(FLAG.linkTimeBack)
			handleData.linkTime=FLAG.linkTimeBack;

			if ( p[COM_COM_OFF] == 0x51 )
			{
				/* Debug_Printf(5,"心跳帧号：%d\r\n",p[COM_FRAME_OFF]); */
			}else if ( p[COM_COM_OFF] == 0x91 )
			{
				/* MY_DEBUG("应答CCID\r\n",buf); */
			}else if ( p[COM_COM_OFF] == COM_DEVICE_USART + 1 )
			{
				/* Debug_Printf(5,"应答联网错误\r\n",buf); */
			}  else if ( p[COM_COM_OFF] == 0X83 )
			{
				/* Debug_Printf(5,"应答GPS\r\n",buf); */
			}
		}
#endif


		if ( p[COM_COM_OFF] == COM_DEVICE_USART + 1 )
		{
		} else if ( p[COM_COM_OFF] == 0x61 ) /* ???? */

		{ /* MY_MEMSET( UpdateBuf,0,sizeof(UpdateBuf)); */
			if ( p[COM_COM_OFF + 1] == 0x00 )
			{
				/*	com_user_send(COM485_GETCODE+0x80,&p[COM_COM_OFF+3],p[COM_COM_OFF+2]); */
			}else if ( p[COM_COM_OFF + 1] == 0x01 )
			{
				/* com_user_send(COM485_GETAD+0x80,&p[COM_COM_OFF+3],p[COM_COM_OFF+2]); */
			}
		}
	}

	/* r = 1; */

	return(len);
}


/***************************************************************************/
#if 0
u8 AnalyseData( u8 *pInAndOut, u8 len, HandleData_sut * pHandleData )
{
	if ( (COM_DATA_HEAD != pInAndOut[0]) || (0 >= len) )
		return(0);

	switch ( pInAndOut[COM_CMD_OFF] )
	{
	case  COM_LOGIN: /* 登录 */
	{
		u32 Versoin;
		len	= COM_LOGIN_LEN;
		Versoin = COMM_VERSOIN;
		MY_MEMCPY( pInAndOut + COM_DATA_OFF, &Versoin, 4 );

#if TEST_DEBUG_HANDLE
		MY_DEBUG( "id==%s,pswd==%s\r\n", pHandleData->commTaskData.id, pHandleData->commTaskData.password );
#endif

		MY_MEMCPY( &pInAndOut[10], pHandleData->commTaskData.id, ID_LEN );
		MY_MEMCPY( &pInAndOut[18], pHandleData->commTaskData.password, PSW_LEN );
		MY_MEMCPY( &pInAndOut[26], &pHandleData->CoinType.CoinScan_Up, 4 );

		Versoin = DEVICE_VERSOIN;
		MY_MEMCPY( pInAndOut + 30, &Versoin, 4 );       /* //???? */

		break;
	}

	case  COM_SET:                                          /* 设置板子参数。 */
	{
		u16 TO;
		len	= COM_SET_LEN;
		TO	= pInAndOut[COM_DATA_OFF + 2] | pInAndOut[COM_DATA_OFF + 3] << 8;
		MY_DEBUG( "Heardbeat timeout==%d\r\n", TO );
		if ( TO < HEARTBEAT_TIME_OUT_MAX )
		{
			TO = HEARTBEAT_TIME_OUT_MAX;
		}
		pHandleData->HeartBeatTimeOut = TO / 2; /* 心跳超时时间( 此时间内收不到心跳包表示连接超时) */
		MY_DEBUG( "HeartBeatTimeOut==%d\r\n", pHandleData->HeartBeatTimeOut );

		MY_MEMSET( pInAndOut + COM_DATA_OFF, 0, 4 );

		break;
	}

	case  COM_STATE:        /* 后台获取板子信号强度。 */
		len = COM_STATE_LEN;
		MY_MEMSET( pInAndOut + COM_DATA_OFF, 0, 4 );
		MY_MEMCPY( pInAndOut + COM_DATA_OFF, &pHandleData->rssi, 1 );
		break;

	case  COM_SET_ID:       /* 设置板子ID和密码。 */
		len = COM_SET_ID_LEN;
		MY_MEMCPY( pHandleData->commTaskData.id, pInAndOut + COM_DATA_OFF, 8 );
		MY_MEMCPY( pHandleData->commTaskData.password, pInAndOut + COM_DATA_OFF + 8, 8 );
		MY_MEMSET( pInAndOut + COM_DATA_OFF, 0, len - COM_DATA_OFF );
		pInAndOut[COM_DATA_OFF] = 1;
		/* ======此处还要添加对id和password的存储。 */
		break;

	case  COM_GET_HARD:                                     /* 获取硬件资讯。 */
		len = COM_GET_HARD_LEN;
		MY_DEBUG( "<-- pInAndOut[COM_DATA_OFF]=%d-->\r\n", pInAndOut[COM_DATA_OFF] );
		if ( pInAndOut[COM_DATA_OFF] == 1 )             /* 硬件CHIP_ID. */
		{
			u32 id = 0;                             /* system_get_chip_id(); */
			MY_MEMSET( pInAndOut + COM_DATA_OFF, 0, len - COM_DATA_OFF );
			pInAndOut[COM_DATA_OFF]		= 1;
			pInAndOut[COM_DATA_OFF + 1]	= 4;
			MY_MEMCPY( pInAndOut + COM_DATA_OFF + 2, &id, pInAndOut[COM_DATA_OFF + 1] );
		}else if ( pInAndOut[COM_DATA_OFF] == 0 )       /* 網卡地址MAC. */
		{
			MY_MEMSET( pInAndOut + COM_DATA_OFF, 0, len - COM_DATA_OFF );
			pInAndOut[COM_DATA_OFF]		= 0;
			pInAndOut[COM_DATA_OFF + 1]	= 6;
		}else if ( pInAndOut[COM_DATA_OFF] == 2 )       /* SIM卡IMSI號. */
		{
			MY_MEMSET( pInAndOut + COM_DATA_OFF, 0, len - COM_DATA_OFF );
			pInAndOut[COM_DATA_OFF]		= 2;
			pInAndOut[COM_DATA_OFF + 1]	= 8;
			MY_MEMCPY( pInAndOut + COM_DATA_OFF + 2, pHandleData->ismi, pInAndOut[COM_DATA_OFF + 1] );
		}else if ( pInAndOut[COM_DATA_OFF] == 3 )       /* CCID号 */
		{
			MY_MEMSET( pInAndOut + COM_DATA_OFF, 0, len - COM_DATA_OFF );
			pInAndOut[COM_DATA_OFF]		= 3;
			pInAndOut[COM_DATA_OFF + 1]	= 10;
			MY_MEMCPY( pInAndOut + COM_DATA_OFF + 2, pHandleData->iccid, pInAndOut[COM_DATA_OFF + 1] );
		}else  {
			MY_MEMSET( pInAndOut + COM_DATA_OFF, 0, len - COM_DATA_OFF );
			pInAndOut[COM_DATA_OFF]		= 0xff;
			pInAndOut[COM_DATA_OFF + 1]	= 0;
		}
		break;

	case  COM_CMD: /* 控制台命令 */
		len = COM_CMD_LEN;
		if ( MY_STRCMP( "_restart", (const char *) pInAndOut + COM_DATA_OFF ) == 0 )
		{
			MY_MEMSET( pInAndOut + COM_DATA_OFF, 0, len - COM_DATA_OFF );
			MY_MEMCPY( pInAndOut + COM_DATA_OFF, "_ok", MY_STRLEN( "_ok" ) );
			pHandleData->systemResetFlag = TRUE;
		}else if ( MY_STRCMP( "_restore", (const char *) pInAndOut + COM_DATA_OFF ) == 0 )
		{
			MY_MEMSET( pInAndOut + COM_DATA_OFF, 0, len - COM_DATA_OFF );
			MY_MEMCPY( pInAndOut + COM_DATA_OFF, "unknown", MY_STRLEN( "unknown" ) );
		}else if ( pInAndOut[COM_DATA_OFF] == '*' )     /* //控台设置命令，出币退奖脉冲宽度间隔等。 */
		{
			u8	CmdLen;
			u8	CmdType;
			u8	CmdData;
			CmdLen = MY_STRLEN( (const char *) (pInAndOut + COM_DATA_OFF + 1) );
			if ( CmdLen < CONSOLE_CMD_MIN_LEN )     /* //判断长度。 */
			{
				CmdType = 0;
				CmdData = 0;
				MY_DEBUG( "GPRS set data len error!\r\n" );
			}else  {
				MY_DEBUG( "GPRS set data:%s\r\n", pInAndOut + COM_DATA_OFF + 1 );

				/* sscanf成功则返回参数数目，失败则返回-1. */
				if ( 2 == MY_SSCANF( (char *) (pInAndOut + COM_DATA_OFF + 1), "%u,%u", &CmdType, &CmdData ) )
				{
					/* ======ConsoleSetData( CmdType,CmdData ); */
				}else  {
					CmdType = 0;
					CmdData = 0;
					MY_DEBUG( "GPRS set data convert error!\r\n" );
				}
			}
			{
				u8 DataTemp[COM_CMD_LEN];
				MY_MEMSET( DataTemp, 0, COM_CMD_LEN );
				MY_SPRINTF( (char *) DataTemp, "%d,0x%02X%02X\0", CmdType, (u8) (CmdData >> 8), (u8) CmdData );
				MY_MEMSET( pInAndOut + COM_DATA_OFF, 0, len - COM_DATA_OFF );
				MY_MEMCPY( pInAndOut + COM_DATA_OFF, DataTemp, MY_STRLEN( (char *) DataTemp ) );
			}
		}else if ( pInAndOut[COM_DATA_OFF] == '&' ) /* //控台查询命令，出币退奖脉冲宽度间隔等。 */
		{
			u8	typeData;
			u16	retData;
			u8	tempData[COM_CMD_LEN];
			MY_MEMSET( tempData, 0, COM_CMD_LEN );

			MY_DEBUG( "GPRS inquire data:%s\r\n", pInAndOut + COM_DATA_OFF + 1 );
			typeData = MY_ATOI( (const char *) (pInAndOut + COM_DATA_OFF + 1) );
			/* ======retData = ConsoleInquireData( typeData ); */
			MY_DEBUG( "inquire Cmd:%d---inquire Data:%d.\r\n", typeData, retData );

			MY_SPRINTF( (char *) tempData, "%d,%d\0", typeData, retData );
			MY_MEMSET( pInAndOut + COM_DATA_OFF, 0, len - COM_DATA_OFF );
			MY_MEMCPY( pInAndOut + COM_DATA_OFF, tempData, MY_STRLEN( (char *) tempData ) );
		}else  {
			MY_MEMSET( pInAndOut + COM_DATA_OFF, 0, len - COM_DATA_OFF );
			MY_MEMCPY( pInAndOut + COM_DATA_OFF, "unknown", MY_STRLEN( "unknown" ) );
		}
		break;

	case  COM_LOGIN_RES:                                                                                    /* 登入結果。////0:登录失败，1：设备禁用，2：账号密码有误。 */
		len = COM_LOGIN_RES_LEN;
		MY_MEMSET( pInAndOut + COM_DATA_OFF, 0, len - COM_DATA_OFF );
		break;

	case  COM_COIN_SCAN:                                                                                    /* 派币 */
		pHandleData->CoinType.CoinScan += pInAndOut[COM_DATA_OFF] | pInAndOut[COM_DATA_OFF + 1] << 8;   /* //派币给维码器。 */
#if TEST_DEBUG_HANDLE
		MY_DEBUG( "CoinScan:%d\n", pHandleData->CoinType.CoinScan );
#endif

		len = COM_COIN_LEN;
		MY_MEMCPY( pInAndOut + COM_DATA_OFF, &pHandleData->CoinType.CoinScan_Up, 4 );                   /* //上报已经上的扫码币.					// */

		break;

	case  COM_COIN_IN:                                                                                      /*上报投退币账目。 */


		/* len = COM_COIN_IN_LEN;
		 * MY_MEMSET(pInAndOut + COM_DATA_OFF,  0, len - COM_DATA_OFF);
		 * MY_MEMCPY(pInAndOut + COM_DATA_OFF,     &pHandleData->CoinType.CoinIn_Up, 2);
		 * MY_MEMCPY(pInAndOut + COM_DATA_OFF + 2, &pHandleData->CoinType.PrizePulseIn_Up, 2);
		 */

		len = COM_COIN_IN_LEN;
		MY_MEMSET( pInAndOut + COM_DATA_OFF, 0, len - COM_DATA_OFF );
		MY_MEMCPY( pInAndOut + COM_DATA_OFF, &device.commTaskData.bill.AllCoinIn, 2 );
		MY_MEMCPY( pInAndOut + COM_DATA_OFF + 2, &device.commTaskData.bill.AllExtendIn, 2 );
		MY_MEMCPY( pInAndOut + COM_DATA_OFF + 4, &device.commTaskData.bill.AllExtendVirtual, 2 );
		break;

	case  COM_FOTA:
		len = COM_FOTA_LEN;

		Fota_DataHandle( pInAndOut );

		MY_MEMSET( pInAndOut + COM_DATA_OFF, 0, len - COM_DATA_OFF );

		pInAndOut[COM_DATA_OFF] = 1;

		break;

	default:
		len = COM_ANSWER_MIN_LEN;
		MY_MEMSET( pInAndOut + COM_DATA_OFF, 0, len - COM_DATA_OFF );
		break;
	} /* end swithc. */

	{
		u16	checkOut = 0;
		u8	i;
		pInAndOut[COM_CMD_OFF]++; /* //应答命令+1； */
		pInAndOut[COM_LEN_OFF] = len;

		for ( i = COM_FRAME_OFF; i < len; i++ )
		{
			checkOut += pInAndOut[i];
		}
		/* MY_DEBUG( "Data check0==%02X%02X\r\n",(u8)(checkOut >> 8), (u8)checkOut ); */
		pInAndOut[COM_CHECK_OFF]	= (u8) checkOut;
		pInAndOut[COM_CHECK_OFF + 1]	= (u8) (checkOut >> 8);
	}

	return(len);
}


#endif

