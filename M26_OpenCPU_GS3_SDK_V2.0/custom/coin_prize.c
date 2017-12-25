

#include "ql_stdlib.h"
#include "ql_type.h"
#include "ql_uart.h"
#include "ql_gpio.h"
#include "ql_error.h"
#include "ql_timer.h"

#include "custom_common.h"
#include "my_stdlib.h"
#include "coin_prize.h"

#include "config.h"


#define COIN_TEST_DEBUG   1
 u16 messageCnt=0;
   u8 powerFirst=0;
  u16 restartcount=0;
  u16 dealy_time;
u8 LED_STATE;



/***IOdata_sut初始化***/
IOdata_sut   ConsoleSet = 
{    
  50,//u8 CoinUp_POW;     //上币脉冲输出宽度.
  100,//u8 PrizeOut_POW;   //退奖脉冲输出宽度 
    
  100,//u8 CoinUp_POI;      //上币脉冲输出间隔。
  200,//u8 PrizeOut_POI;    //退奖脉冲输出间隔。
  
  25,//u8 CoinIn_PIWmin;    //投币器脉冲输入宽度最小值.
  1000,//u8 CoinIn_PIWmax;    //投币器脉冲输入宽度最大值.
  
  25,//u8 PrizeOut_PIWmin;  //退奖脉冲输入宽度最小值.
  1000,//u8 PrizeOut_PIWmax;  //退奖脉冲输入宽度最大值.
  
  30,//u8 CoinIn_PIImin;     //投币器脉冲输入最小间隔.
  250,//u8 PrizeOut_PIImin;   //退奖脉冲输入最小间隔.
  
  0,//u8 PrizeOut_NONC;   //退奖信号常开常闭.
  0,//u8 LocalSaveFlag;   //本地保存标志.
  0 //u8 CoinUp_NONC;     //上币信号常开常闭.  
};

u8 g_coinUpTimeAcc = 0;

PLUSE_MODE g_CoinIn_NONC = OPEN;
u16  g_coinInPulseWidthTime    = 0;      //脉冲宽度计时。
u8   g_coinInPulseIntervalTime = 0;      //脉冲间隔计时。
u8   g_CoinInPulseStartFlag    = FALSE;  //投币脉冲结束标志。
u8    coinInState1=0;
//u16  gReadCoinCnt=0;

u8 g_back_prize_TimeAcc=0;

u16  g_prizePulseWidthTime    = 0;      //脉冲宽度计时。
u8   g_prizePulseIntervalTime = 0;      //脉冲间隔计时。
u8   g_prizePulseStartFlag    = FALSE;  //投币脉冲结束标志。
//u8    prizeState1=0;
//u16  gReadCoinCnt=0;




#if RESEND==1
void LinkfFalutSend(void)
{		
	u8 i=0;
	u8 buf_temp[64]={0};
		for(i=0;i<8;i++){
			if(FLAG.linkfFalut[i])break;
		}
		if(i!=8){//有数据需要提交
				((u8*)buf_temp)[0]=0xff;
				((u8*)buf_temp)[1]=0x02;
				((u8*)buf_temp)[2]=0x01;			               
				((u8*)buf_temp)[3]=32;
				FLAG.ACKsend=1;
				FLAG.ACKsendTimeout=10*OS_TICKS_PER_SEC;
				Ql_sprintf((u8*)buf_temp+4, "%d,%d,%d,%d,%d,%d,%d,%d",FLAG.linkfFalut[0],FLAG.linkfFalut[1],FLAG.linkfFalut[2],FLAG.linkfFalut[3],FLAG.linkfFalut[4],FLAG.linkfFalut[5],FLAG.linkfFalut[6],FLAG.linkfFalut[7]);
				send_data_to_server(0x82, (u8*)buf_temp, sizeof(buf_temp));
		   	FLAG.deviceSendBackup=5;
							  
				FLAG.ACKsend=0;
				FLAG.REGFalut=0;	
				FLAG.simFalut=0;
				FLAG.HeartFalut=0;
				FLAG.reconnetCount=0;
				FLAG.reconnetWWWCount=0;
				APP_DEBUG("%d,%d,%d,%d,%d,%d,%d,%d",FLAG.linkfFalut[0],FLAG.linkfFalut[1],FLAG.linkfFalut[2],FLAG.linkfFalut[3],FLAG.linkfFalut[4],FLAG.linkfFalut[5],FLAG.linkfFalut[6],FLAG.linkfFalut[7]);
				Ql_memset( FLAG.linkfFalut,0,sizeof(FLAG.linkfFalut));
					//Debug_Printf(5,"SEND DEVICE_LINKERR\r\n");
		}
}

void GPSSend(void)
{		
	u8 i=0;
	u8 buf_temp[64]={0};
	Ql_memset(buf_temp,0,sizeof(buf_temp));
	if(Ql_strlen(FLAG.longlat))
	{
		((u8*)buf_temp)[0]=0xff;
		((u8*)buf_temp)[1]=0x01;
		#if loction==1
		//((u8*)buf_temp)[2]=0x01;//之前gps用的1，现在用2		
		((u8*)buf_temp)[2]=0x02;
		#endif           
		((u8*)buf_temp)[3]=Ql_strlen(FLAG.longlat);
	
		Ql_sprintf((u8*)buf_temp+4,"%s", FLAG.longlat);
		send_data_to_server(0x82, (u8*)buf_temp, Ql_strlen(buf_temp));
		Ql_memset(FLAG.longlat,0,sizeof(FLAG.longlat))	;
	}    
}


void SysFalutSend(void)
{

				
}


u8 random=0;
void Device_CoinIn_Send(void)
{
	u8 buf_temp[16]={0};
	Ql_memcpy(buf_temp, &device.commTaskData.bill.AllCoinIn,6);
  ((u8*)buf_temp)[6]=FLAG.Rssi;
  //send_data_to_server(0x50, (u8*)buf_temp, 8);
APP_DEBUG("coin_heart:%d,%d,%d,%d,%d\r\n",device.commTaskData.bill.AllCoinIn,
device.commTaskData.bill.AllExtendIn,device.commTaskData.bill.AllExtendVirtual,
FLAG.Rssi,random);

  send_data_to_server1(0x50, (u8*)buf_temp, 8,random++);

}


void Device_CCID_Send(void)
{			
 	u8 buf_temp[32]={0};
  ((u8*)buf_temp)[0]=3;
  ((u8*)buf_temp)[1]=10;
	Ql_memcpy(&buf_temp[2], FLAG.iccid, 10);
  send_data_to_server(0x90, (u8*)buf_temp, 12);
}


void Device_Heart_Send(void)
{
	
		if(FLAG.ToServerFlag)
		{      
			if((FLAG.FirstPower&0x80)==0)
			{
         
			  switch(FLAG.WhichToSend)
				{			
					case DEVICE_CCID:
					if((FLAG.FirstPower&0x01)==0)
					{
						APP_DEBUG("SEND DEVICE_CCID\r\n");	
						Device_CCID_Send();
						FLAG.FirstPower|=0x01;
						FLAG.WhichToSend=DEVICE_COIN;
					}
					break;
					
					case DEVICE_COIN:
					Device_CoinIn_Send();
					FLAG.WhichToSend=DEVICE_GPS;
					APP_DEBUG("SEND DEVICE_COIN\r\n");	
					break;
					
					
					case DEVICE_GPS:
					GPSSend();	
					APP_DEBUG("SEND DEVICE_GPS\r\n");
					FLAG.WhichToSend=DEVICE_LINKERR;
					break;
					
					case DEVICE_LINKERR:
						LinkfFalutSend();
					FLAG.WhichToSend=DEVICE_COIN;
					FLAG.FirstPower|=0x80;
						APP_DEBUG("SEND DEVICE_LINKERR FIRST\r\n");	
					break;

				//	case DEVICE_SYSERR:SysFalutSend();;
					
					default:break;
				}
				FLAG.deviceSendBackup=5;
				
     }
		 else
		 {			
			 LinkfFalutSend();

			 if(FLAG.deviceSendBackup==0)
			 {
				 Device_CoinIn_Send();
				 FLAG.deviceSendBackup=FLAG.deviceSendTime;
			 }
						
     }
  FLAG.ToServerFlag=0;
 }

}

#endif




void Callback_LED(u32 timerId, void* param)
{

   if (LED ==timerId)
   	{
		LED_WORK(LED_STATE);
		 Back_prize();
   	}
		


  


}



void Bee_handle(void)
{
		

 }

#define BUZZER_OPEN_TIME 40
#define BUZZER_CLOSE_TIME 80
void StartBuzzer(u8 count)
{
  FLAG.Report_Count = count;
  FLAG.BuzzerInv = BUZZER_CLOSE_TIME;
  if (FLAG.CloseBuzzerTime)
    return ;
  COIN_UP_GPIO_OUT_SET(1);//0;
  FLAG.CloseBuzzerTime = BUZZER_OPEN_TIME;
}

void led_on(void)
{
	COIN_UP_GPIO_OUT_SET(0);//1;	

}

void led_off(void)
{
	COIN_UP_GPIO_OUT_SET(1);//1;	

}

void led_handle(u16 cnt, u8 time)
{
	if(cnt<=(time*2-1)*LED_CNT){
		if(cnt%LED_CNT==0){
			if((cnt/LED_CNT)%2==0){
				led_on();
			}else{
				led_off();
			}
		}
	}
}


void LED_WORK(u8 state)
{

	static u16	ledCnt=0;
	static u16	led=0;


			switch (state) {
				case LED_STATE_NO_LINK:
			
				if (ledCnt%5 == 0) {
					if(!led)
				      led_on();
					else
					  led_off();	
					}

			break;
			
			case LED_STATE_STOP_LINK:
			#if RESEND==1
			FLAG.deviceSendBackup=0;
			#endif
				led_handle(ledCnt,3);

				break;
			case LED_STATE_READ_ERR:
				led_handle(ledCnt,4);
				break;
			case LED_STATE_WORK:
			#if KANGTAI_EN==1
			if (ledCnt%50 == 0) {
					if(!led)
				      led_on();
					else
					  led_off();	
					}

		#else
			led_on();
		#endif
		
		break;
				
			case LED_STATE_GPRS_UPDATA:
			//led_handle(ledCnt,4);
				break;
				


	case LED_STATE_GPRS_UPDATA_ERR: 								
		#if _20170120==1            
			//  led_handle(ledCnt,2);   
        #endif
				break;
			
			}



}




void Back_prize(void)
{


if( g_CoinIn_NONC == CLOSE )	//退奖常闭
 {
	 if( COIN_INPUT_CHECK && !g_prizePulseStartFlag )	//有脉冲
	 {	  
		 g_prizePulseIntervalTime = 0;  
		 g_prizePulseWidthTime = 0;
		 g_prizePulseStartFlag = 1;
	 }
	 else if( !COIN_INPUT_CHECK && g_prizePulseStartFlag ) 	//脉冲结束。
	 {					 
		 g_prizePulseStartFlag = 0; 

		 #if  COIN_TEST_DEBUG  
		 MY_DEBUG( "Close:g_coinInPulseWidthTime==%dms\r\n",g_prizePulseWidthTime*LED_TIMER_PERIOD );
		 #endif
	 }
 }
else 	//退奖常开
{
	 if( !COIN_INPUT_CHECK && !g_prizePulseStartFlag )  //有脉冲
	 {	  
		 g_prizePulseIntervalTime = 0;  
		 g_prizePulseWidthTime = 0;
		 g_prizePulseStartFlag = 1;
	 }
	 else if( COIN_INPUT_CHECK && g_prizePulseStartFlag )	   //脉冲结束。
	 {					 
		 g_prizePulseStartFlag = 0;  
		 #if  COIN_TEST_DEBUG  
		 MY_DEBUG( "Open:g_coinInPulseWidthTime==%dms\r\n",g_prizePulseWidthTime*LED_TIMER_PERIOD );
		 #endif
	 }    
}

//脉冲宽度检测/
if( g_prizePulseStartFlag )  //有脉冲，开始计时。
{
	g_prizePulseWidthTime++;
	if( g_prizePulseWidthTime*LED_TIMER_PERIOD > device.commTaskData.ioPara.plusePara[PLUSE_IN_W_MAX1] )  ////超出常开常闭检测时间。
	{
		 if( g_CoinIn_NONC == CLOSE )
		 {
			g_CoinIn_NONC = OPEN;
		 }
		 else if( g_CoinIn_NONC == OPEN )
		 {
			g_CoinIn_NONC = CLOSE;
		 }
		 g_prizePulseIntervalTime = 0;  
		 g_prizePulseWidthTime = 0;
		 g_prizePulseStartFlag = 0;
	}
}
else	   //脉冲结束。
{
	
	if(    g_prizePulseWidthTime*LED_TIMER_PERIOD > device.commTaskData.ioPara.plusePara[PLUSE_IN_W_MIN1] 
		&& g_prizePulseWidthTime*LED_TIMER_PERIOD < device.commTaskData.ioPara.plusePara[PLUSE_IN_W_MAX1] )
	{ 
		g_prizePulseIntervalTime++;   //
	}
	if( g_prizePulseIntervalTime*LED_TIMER_PERIOD > device.commTaskData.ioPara.plusePara[PLUSE_IN_W_MIN1] )
	{
				 FLAG.CoinPlusCount=g_prizePulseWidthTime*LED_TIMER_PERIOD;
	   handleData.CoinType.PrizePulseIn++;		
	   g_prizePulseIntervalTime = 0;  
	   g_prizePulseWidthTime = 0;
	   #if	COIN_TEST_DEBUG  
	   
	   MY_DEBUG( "g_coinInPulseWidthTime==%dms\r\n",FLAG.CoinPlusCount );
	   MY_DEBUG( "CoinType.CoinIn==%d\r\n",handleData.CoinType.PrizePulseIn );
	   #endif
	} 
	}


if( handleData.CoinType.PrizePulseIn )  //退奖。
	 {	  
		 if( handleData.CoinType.PrizePulseIn > device.commTaskData.ioPara.plusePara[MAX_UP_COIN] )
		 {
		   handleData.CoinType.CoinIn = 0;
		   return;
		 }
		 if( OPEN == ConsoleSet.PrizeOut_NONC  ) //常开，平时为低电平。
		 {	 
		      g_back_prize_TimeAcc++;
			 g_coinUpTimeAcc++;
			 if( g_back_prize_TimeAcc*LED_TIMER_PERIOD <= device.commTaskData.ioPara.plusePara[PLUSE_W1]  )
			 {
				 COIN_UP_GPIO_OUT_SET(1);//1;				 
			 }			
			 else 
			 {
				 COIN_UP_GPIO_OUT_SET(0);//0;
				 if( g_back_prize_TimeAcc*LED_TIMER_PERIOD > device.commTaskData.ioPara.plusePara[PLUSE_W1] + device.commTaskData.ioPara.plusePara[PLUSE_S1] )
				 {
					 g_back_prize_TimeAcc = 0;
					 handleData.CoinType.PrizePulseIn--;
					 handleData.CoinType.PrizePulseIn_Up++; 
                     #if  COIN_TEST_DEBUG    
					 MY_DEBUG( "OPEN CoinIn==%d\r\n", handleData.CoinType.PrizePulseIn );
                     #endif
				 }
			 } 
		 }
		 else if( CLOSE == ConsoleSet.PrizeOut_NONC  )//常闭，平时为高电平。
		 {	 
			 g_back_prize_TimeAcc++;
			 if( g_back_prize_TimeAcc*LED_TIMER_PERIOD <= device.commTaskData.ioPara.plusePara[PLUSE_W1] )
			 {
				 COIN_UP_GPIO_OUT_SET(0);//0;				 
			 }
			 else 
			 {
				 COIN_UP_GPIO_OUT_SET(1);//1;
				 if( g_back_prize_TimeAcc*LED_TIMER_PERIOD > device.commTaskData.ioPara.plusePara[PLUSE_W1] + device.commTaskData.ioPara.plusePara[PLUSE_S1] )
				 {
					   g_back_prize_TimeAcc = 0;   
					  handleData.CoinType.PrizePulseIn--;
					 handleData.CoinType.PrizePulseIn_Up++; 
                       #if  COIN_TEST_DEBUG    
					   MY_DEBUG( "CLOSE CoinIn==%d\r\n",  handleData.CoinType.PrizePulseIn );
                       #endif
				 }
			 }				
		 }		  
	 }		




}

void bill_Write(u32 wirteAddr,Bill_sut *pNewBill, Bill_sut *pOldBill)
{
	Ql_SecureData_Store((wirteAddr%5)+1, (u8*)&device.commTaskData.bill, sizeof(Bill_sut)); 

	
}
void BubbleSort2(unsigned int *arr,unsigned int sz){
    unsigned int i = 0;
    unsigned int j = 0;
  unsigned int tmp=0;
  
 // APP_DEBUG("<--arr4:%d %d %d %d %d %d -->\r\n",arr[0],arr[1],arr[2],arr[3],arr[4],arr[5]);
    for(i=0;i<sz-1;i++){
        for(j=sz-1;j>i;j--){
            if(arr[j]>arr[j-1]){
                 tmp = arr[j];
                arr[j] = arr[j-1];
                arr[j-1] = tmp;

				
			//	APP_DEBUG("<--arr3:%d %d %d %d %d %d -->\r\n",arr[0],arr[1],arr[2],arr[3],arr[4],arr[5]);
            }
        }
    }
}

void bill_Read(void)
{

  char i=0;
  unsigned int arr[6]={0};
  unsigned int arr1[6]={0};
  Bill_sut pNewBill;
  u8 ress;
   
  for(i=1;i<6;i++)
  {
     Ql_SecureData_Read(i, (u8*)&pNewBill, sizeof(Bill_sut)); 

     arr[i-1]=pNewBill.countFlag;

	 APP_DEBUG("<--device.commTaskData.bill:%d %d %d %d %d %d -->\r\n",i,pNewBill.countFlag,
	pNewBill.AllCoin,
	pNewBill.AllCoinIn,
	pNewBill.AllExtendIn,
	pNewBill.AllExtendVirtual);
  }
  
    APP_DEBUG("<--arr:%d %d %d %d %d %d -->\r\n",arr[0],arr[1],arr[2],arr[3],arr[4],arr[5]);
    Ql_memcpy(arr1,arr,sizeof(arr1));

	
	//APP_DEBUG("<--arr1:%d %d %d %d %d %d -->\r\n",arr1[0],arr1[1],arr1[2],arr1[3],arr1[4],arr1[5]);
	BubbleSort2(arr1,6);
	
	APP_DEBUG("<--arr2:%d %d %d %d %d %d -->\r\n",arr1[0],arr1[1],arr1[2],arr1[3],arr1[4],arr1[5]);
	for(i=0;i<6;i++)
	{
       if(arr1[0]==arr[i])
       	{
          Ql_SecureData_Read(i+1, (u8*)&device.commTaskData.bill, sizeof(Bill_sut)); 
		  APP_DEBUG("<--跳出:%d %d %d %d %d %d -->\r\n",i,device.commTaskData.bill.countFlag,
	      device.commTaskData.bill.AllCoin,
	      device.commTaskData.bill.AllCoinIn,
	      device.commTaskData.bill.AllExtendIn,
	      device.commTaskData.bill.AllExtendVirtual);
		  break;
	    }
	}
	
    
}

/*************************************************************************************/
/*************************************************************************************/
void gpioInit(void)
{
    s32 ret = -1;
	
	/*****************************/
	ret = Ql_GPIO_Uninit( COIN_UP_PIN ); 
	if( QL_RET_OK == ret )
	{
		MY_DEBUG( "<-- COIN_UP_PIN uninit successfully -->\r\n" ); 
	}
	else
	{
		MY_DEBUG( "<-- COIN_UP_PIN uninit failure:%d -->\r\n", ret ); 
	}
	ret = Ql_GPIO_Init( COIN_UP_PIN, PINDIRECTION_OUT, PINLEVEL_LOW, PINPULLSEL_PULLUP ); 	 
	if( QL_RET_OK == ret )
	{
		MY_DEBUG( "<-- COIN_UP_PIN init successfully -->\r\n" ); 
	}
	else
	{
		MY_DEBUG( "<-- COIN_UP_PIN init failure:%d -->\r\n", ret ); 
	}

	/*****************************/
	ret = Ql_GPIO_Init( COIN_IN_PIN, PINDIRECTION_IN, PINLEVEL_HIGH, PINPULLSEL_PULLUP ); 	 
	if( QL_RET_OK == ret )
	{
		MY_DEBUG( "<-- COIN_IN_PIN init successfully -->\r\n" ); 
	}
	else
	{
		MY_DEBUG( "<-- COIN_IN_PIN init failure:%d -->\r\n", ret ); 
	}


    ret = Ql_GPIO_Init( COIN_PRIZE_PIN, PINDIRECTION_IN, PINLEVEL_HIGH, PINPULLSEL_PULLUP ); 	 
	if( QL_RET_OK == ret )
	{
		MY_DEBUG( "<-- COIN_PRIZE_PIN init successfully -->\r\n" ); 
	}
	else
	{
		MY_DEBUG( "<-- COIN_PRIZE_PIN init failure:%d -->\r\n", ret ); 
	}
	
	/*****************************/
	#if 0
	ret = Ql_GPIO_Init( NETLIGHT_PIN, PINDIRECTION_OUT, PINLEVEL_LOW, PINPULLSEL_PULLUP ); 	 
	if( QL_RET_OK == ret )
	{
		MY_DEBUG( "<-- NETLIGHT_PIN init successfully -->\r\n" ); 
	}
	else
	{
		MY_DEBUG( "<-- NETLIGHT_PIN init failure:%d -->\r\n", ret ); 
	}	
	#endif
}

/*************************************************************************************/
/*************************************************************************************/
/*
PLUSE_MODE g_CoinIn_NONC = OPEN;
u16  g_coinInPulseWidthTime    = 0;      //脉冲宽度计时。
u8   g_coinInPulseIntervalTime = 0;      //脉冲间隔计时。
u8   g_CoinInPulseStartFlag    = FALSE;  //投币脉冲结束标志。
u8    coinInState1=0;
u16  gReadCoinCnt=0;
*/
void Callback_CoinInPulseCheck( u32 timerId, void* param )
{
/*
   static u16 messageCnt=0;
   static u8 powerFirst=0;
   static u8 restartcount=0;

*/
    if ( COIN_IN_TIMER_ID != timerId )
	return;
	/*
	if(handleData.CoinType.CoinScan)
		return ;
	else
	*/
		{

	if(FLAG.Ready==0)return;

	if(get_bitFor(handleData.bitFlag, BIT_RESTART))
	{
	MY_DEBUG("restartcount=%d\r\n",restartcount );
      if(restartcount++>400)
      {
     // MY_DEBUG("restartcount=%d\r\n",restartcount );
		if(handleData.CoinType.CoinScan||handleData.CoinType.CoinIn||handleData.CoinType.PrizePulseIn)
		{
           MY_DEBUG("<<<-- bill is not 0......-->>>\r\n" ); 

		}
		else
		{
            MY_DEBUG(" restart=%d\r\n",restart ); 
		
		    handleData.bitFlag=set_bitFor(handleData.bitFlag, BIT_RESTART,FALSE);
			if(restart)
				{
			restartcount=0;
			restart=0;
			MY_DEBUG(" restart=%d\r\n",restart ); 
			MY_DEBUG("< System reset>\r\n" );
			Ql_Reset(0);
				}
		}


	  }
	}




	

	

	if(powerFirst==0)
	{
	  powerFirst=1;
	  FLAG.CoinStat = COIN_INPUT_CHECK;
	  if(get_bitFor(device.commTaskData.ioPara.set, SET_PLUSE_MODE))FLAG.coinstanormal=0;
	  else FLAG.coinstanormal=1;
	  
	  if(get_bitFor(device.commTaskData.ioPara.set, SET_SOFT_PLUSE_MODE))g_CoinIn_NONC=CLOSE;
	  else g_CoinIn_NONC=OPEN;
	}



	if(messageCnt++>=OS_TICKS_PER_SEC/20){//提交一次数据
	messageCnt=0;
		

	if(handleData.CoinType.CoinScan_Up||handleData.CoinType.CoinIn_Up||handleData.CoinType.PrizePulseIn_Up)
    {
        device.commTaskData.bill.AllCoin+=handleData.CoinType.CoinScan_Up;
		device.commTaskData.bill.AllCoinIn+=handleData.CoinType.CoinIn_Up;
		device.commTaskData.bill.AllExtendIn+=handleData.CoinType.PrizePulseIn;

		
		if(handleData.CoinType.CoinIn_Up||handleData.CoinType.PrizePulseIn)
		{
			if(FLAG.deviceSendBackup>20)
			{
				FLAG.deviceSendBackup=10;//有投币的话10秒后上报，没有一分钟上报一次
			}
			else
			{
				
			}
         }


	
			handleData.CoinType.CoinScan_Up=0;
			handleData.CoinType.CoinIn_Up=0;
			handleData.CoinType.PrizePulseIn=0;


		if(!get_bitFor(device.commTaskData.ioPara.set,SET_LOCAL_SAVE)){//默认保存
			device.commTaskData.bill.countFlag++;
			 MY_DEBUG( "device.commTaskData.bill.countFlag==%d\r\n",device.commTaskData.bill.countFlag );
			bill_Write(device.commTaskData.bill.countFlag,&device.commTaskData.bill, &device.oldBill);
	
		}
		else
		{
  
        }

}

}






	
	 /* FLAG.OldCoinOutSta=COIN_PRIZE_CHECK;
	  if (coinInState1 == 0)
	  {
		  if(FLAG.OldCoinOutSta!=FLAG.coinstanormal)
		  {
			coinInState1=1;
		    MY_DEBUG("coinInState1=1\r\n",FLAG.CoinPlusCount);					  
		  }
	  }else if(coinInState1 == 1)
	  {
	    gReadCoinCnt++;
	    if(FLAG.OldCoinOutSta==FLAG.coinstanormal)
	    {
		
			if((gReadCoinCnt*COIN_IN_TIMER_PERIOD<=device.commTaskData.ioPara.plusePara[PLUSE_IN_W_MAX1])&&(gReadCoinCnt*COIN_IN_TIMER_PERIOD>=device.commTaskData.ioPara.plusePara[PLUSE_IN_W_MIN1]) )
			{
				//FLAG.CoinPlusCount=gReadCoinCnt*COIN_IN_TIMER_PERIOD;
				coinInState1=0;
				gReadCoinCnt = 0;
				handleData.CoinType.PrizePulseIn++;
				//MY_DEBUG("FLAG.CoinPlusCount=%d\r\n",FLAG.CoinPlusCount);
			}
			else
			{
			    coinInState1=0;
			    gReadCoinCnt = 0;
			}
	
	     }
	
			  
	}
*/





    if( g_CoinIn_NONC == CLOSE )	//投币常闭
     {
         if( COIN_INPUT_CHECK && !g_CoinInPulseStartFlag )  //有脉冲
         {    
             g_coinInPulseIntervalTime = 0;  
             g_coinInPulseWidthTime = 0;
             g_CoinInPulseStartFlag = 1;
         }
         else if( !COIN_INPUT_CHECK && g_CoinInPulseStartFlag )     //脉冲结束。
         {                   
             g_CoinInPulseStartFlag = 0; 

             #if  COIN_TEST_DEBUG  
             MY_DEBUG( "Close:g_coinInPulseWidthTime==%dms\r\n",g_coinInPulseWidthTime*COIN_IN_TIMER_PERIOD );
             #endif
         }
     }
    else    //投币常开
    {
         if( !COIN_INPUT_CHECK && !g_CoinInPulseStartFlag )  //有脉冲
         {    
             g_coinInPulseIntervalTime = 0;  
             g_coinInPulseWidthTime = 0;
             g_CoinInPulseStartFlag = 1;
         }
         else if( COIN_INPUT_CHECK && g_CoinInPulseStartFlag )     //脉冲结束。
         {                   
             g_CoinInPulseStartFlag = 0;  
             #if  COIN_TEST_DEBUG  
             MY_DEBUG( "Open:g_coinInPulseWidthTime==%dms\r\n",g_coinInPulseWidthTime*COIN_IN_TIMER_PERIOD );
             #endif
         }
    }
    
    /***脉冲宽度检测***/
    if( g_CoinInPulseStartFlag )  //有脉冲，开始计时。
    {
        g_coinInPulseWidthTime++;
        if( g_coinInPulseWidthTime*COIN_IN_TIMER_PERIOD > device.commTaskData.ioPara.plusePara[PLUSE_IN_W_MAX0] )  ////超出常开常闭检测时间。
        {
             if( g_CoinIn_NONC == CLOSE )
             {
                g_CoinIn_NONC = OPEN;
             }
             else if( g_CoinIn_NONC == OPEN )
             {
                g_CoinIn_NONC = CLOSE;
             }
             g_coinInPulseIntervalTime = 0;  
             g_coinInPulseWidthTime = 0;
             g_CoinInPulseStartFlag = 0;
        }
    }
    else       //脉冲结束。
    {
        
        if(    g_coinInPulseWidthTime*COIN_IN_TIMER_PERIOD > device.commTaskData.ioPara.plusePara[PLUSE_IN_W_MIN0] 
			&& g_coinInPulseWidthTime*COIN_IN_TIMER_PERIOD < device.commTaskData.ioPara.plusePara[PLUSE_IN_W_MAX0] )
        { 
            g_coinInPulseIntervalTime++;   //
        }
        if( g_coinInPulseIntervalTime*COIN_IN_TIMER_PERIOD > device.commTaskData.ioPara.plusePara[PLUSE_IN_W_MIN0] )
        {
        			 FLAG.CoinPlusCount=g_coinInPulseWidthTime*COIN_IN_TIMER_PERIOD;
           handleData.CoinType.CoinIn++;        
           g_coinInPulseIntervalTime = 0;  
           g_coinInPulseWidthTime = 0;
           #if  COIN_TEST_DEBUG  
		   
		   MY_DEBUG( "g_coinInPulseWidthTime==%dms\r\n",FLAG.CoinPlusCount );
           MY_DEBUG( "CoinType.CoinIn==%d\r\n",handleData.CoinType.CoinIn );
           #endif
        } 
    	}
    } 
 }

 /*************************************************************************************/
 /*************************************************************************************/



 void Callback_CoinUpPulseOut( u32 timerId, void* param ) 
 {
     if ( COIN_UP_TIMER_ID != timerId )
	 return;
	// MY_DEBUG( "COIN_UP_TIMER_ID IS TIME OUT" );


	if(get_bitFor(device.commTaskData.ioPara.set, SET_SOFT_PLUSE_MODE))ConsoleSet.CoinUp_NONC=CLOSE;
			else ConsoleSet.CoinUp_NONC=OPEN;
			
	 if( handleData.CoinType.CoinScan )  //扫码上币。
	 {
		 if( handleData.CoinType.CoinScan > device.commTaskData.ioPara.plusePara[MAX_UP_COIN] )
		 {
		   handleData.CoinType.CoinScan = 0;
		   return;
		 }
		 if( OPEN == ConsoleSet.CoinUp_NONC ) //常开，平时为低电平。
		 {	 
			 g_coinUpTimeAcc++;
			 if( g_coinUpTimeAcc*COIN_UP_TIMER_PERIOD <= device.commTaskData.ioPara.plusePara[PLUSE_W0] )
			 {
				 COIN_UP_GPIO_OUT_SET(1);//1;	
			//  MY_DEBUG( "g_coinUpTimeAcc=%d\r\n", g_coinUpTimeAcc );
			 }			
			 else 
			 {
				 COIN_UP_GPIO_OUT_SET(0);//0;
				 if( g_coinUpTimeAcc*COIN_UP_TIMER_PERIOD > device.commTaskData.ioPara.plusePara[PLUSE_W0] + device.commTaskData.ioPara.plusePara[PLUSE_S0] )
				 {			
				//   MY_DEBUG( "g_coinUpTimeAcc=%d\r\n", g_coinUpTimeAcc );
					 g_coinUpTimeAcc = 0;
					 handleData.CoinType.CoinScan--;
					 handleData.CoinType.CoinScan_Up++;   
				     #if  COIN_TEST_DEBUG	 
				     MY_DEBUG( "OPEN CoinScan==%d\r\n",handleData.CoinType.CoinScan );
				     #endif
				 }
			 } 
		 }
		 else if( CLOSE == ConsoleSet.CoinUp_NONC ) //常闭，平时为高电平。
		 {	 
			 g_coinUpTimeAcc++;
			 if( g_coinUpTimeAcc*COIN_UP_TIMER_PERIOD <= device.commTaskData.ioPara.plusePara[PLUSE_W0])
			 {
				 COIN_UP_GPIO_OUT_SET(0);//0;				 
			 }
			 else 
			 {
				 COIN_UP_GPIO_OUT_SET(1);//1;
				 if( g_coinUpTimeAcc*COIN_UP_TIMER_PERIOD > device.commTaskData.ioPara.plusePara[PLUSE_W0] + device.commTaskData.ioPara.plusePara[PLUSE_S0]  )
				 {					   
					   g_coinUpTimeAcc = 0;   
					   handleData.CoinType.CoinScan--;
					   handleData.CoinType.CoinScan_Up++;
                      #if  COIN_TEST_DEBUG    
					   MY_DEBUG( "CLOSE CoinScan==%d\r\n", handleData.CoinType.CoinScan );
                      #endif
				 }
			 }				
		 }
	 }
	 else if( handleData.CoinType.CoinIn )  //投币器上币。
	 {	  
		 if( handleData.CoinType.CoinIn > device.commTaskData.ioPara.plusePara[MAX_UP_COIN] )
		 {
		   handleData.CoinType.CoinIn = 0;
		   return;
		 }
		 if( OPEN == ConsoleSet.CoinUp_NONC  ) //常开，平时为低电平。
		 {	 
			 g_coinUpTimeAcc++;
			 if( g_coinUpTimeAcc*COIN_UP_TIMER_PERIOD <= device.commTaskData.ioPara.plusePara[PLUSE_W0]  )
			 {
				 COIN_UP_GPIO_OUT_SET(1);//1;				 
			 }			
			 else 
			 {
				 COIN_UP_GPIO_OUT_SET(0);//0;
				 if( g_coinUpTimeAcc*COIN_UP_TIMER_PERIOD > device.commTaskData.ioPara.plusePara[PLUSE_W0] + device.commTaskData.ioPara.plusePara[PLUSE_S0] )
				 {
					 g_coinUpTimeAcc = 0;
					 handleData.CoinType.CoinIn--;
					 handleData.CoinType.CoinIn_Up++; 
                     #if  COIN_TEST_DEBUG    
					 MY_DEBUG( "OPEN CoinIn==%d\r\n", handleData.CoinType.CoinIn );
                     #endif
				 }
			 } 
		 }
		 else if( CLOSE == ConsoleSet.CoinUp_NONC  )//常闭，平时为高电平。
		 {	 
			 g_coinUpTimeAcc++;
			 if( g_coinUpTimeAcc*COIN_UP_TIMER_PERIOD <= device.commTaskData.ioPara.plusePara[PLUSE_W0] )
			 {
				 COIN_UP_GPIO_OUT_SET(0);//0;				 
			 }
			 else 
			 {
				 COIN_UP_GPIO_OUT_SET(1);//1;
				 if( g_coinUpTimeAcc*COIN_UP_TIMER_PERIOD > device.commTaskData.ioPara.plusePara[PLUSE_W0] + device.commTaskData.ioPara.plusePara[PLUSE_S0] )
				 {
					   g_coinUpTimeAcc = 0;   
					   handleData.CoinType.CoinIn--;
					   handleData.CoinType.CoinIn_Up++;
                       #if  COIN_TEST_DEBUG    
					   MY_DEBUG( "CLOSE CoinIn==%d\r\n", handleData.CoinType.CoinIn );
                       #endif
				 }
			 }				
		 }		  
	 }		
/*        退奖       */





	 

	 
 }




















