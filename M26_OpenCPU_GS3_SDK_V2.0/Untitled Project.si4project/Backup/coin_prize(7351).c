

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
  u8 restartcount=0;
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


PLUSE_MODE g_CoinIn_NONC = OPEN;
u16  g_coinInPulseWidthTime    = 0;      //脉冲宽度计时。
u8   g_coinInPulseIntervalTime = 0;      //脉冲间隔计时。
u8   g_CoinInPulseStartFlag    = FALSE;  //投币脉冲结束标志。
u8    coinInState1=0;
u16  gReadCoinCnt=0;


void Callback_LED(u32 timerId, void* param)
{

   if (LED ==timerId)
   	{
		LED_WORK(LED_STATE);
		 Back_prize();
   	}
		


  


}

void LED_WORK(u8 state)
{
  dealy_time++;
switch(state)
  	{
   case SERVER_LINK :
	COIN_UP_GPIO_OUT_SET(1);//1;
   	break;
	case SERVER_NO_LINK :
		COIN_UP_GPIO_OUT_SET(1);//1;
	
	if(dealy_time > 10)
	COIN_UP_GPIO_OUT_SET(0);//0;
	if(dealy_time > 20)
		dealy_time = 0;
		break;
	case SERVER_OTHER_STATE :
		COIN_UP_GPIO_OUT_SET(1);//1;
	
	if(dealy_time > 200)
	COIN_UP_GPIO_OUT_SET(0);//0;
	if(dealy_time > 400)
		dealy_time = 0;
		break;
	default :
		break;
}

}



void Back_prize(void)
{


if( g_CoinIn_NONC == CLOSE )	//投币常闭
 {
	 if( COIN_INPUT_CHECK && !g_CoinInPulseStartFlag )	//有脉冲
	 {	  
		 g_coinInPulseIntervalTime = 0;  
		 g_coinInPulseWidthTime = 0;
		 g_CoinInPulseStartFlag = 1;
	 }
	 else if( !COIN_INPUT_CHECK && g_CoinInPulseStartFlag ) 	//脉冲结束。
	 {					 
		 g_CoinInPulseStartFlag = 0; 

		 #if  COIN_TEST_DEBUG  
		 MY_DEBUG( "Close:g_coinInPulseWidthTime==%dms\r\n",g_coinInPulseWidthTime*COIN_IN_TIMER_PERIOD );
		 #endif
	 }
 }
else 	//投币常开
{
	 if( !COIN_INPUT_CHECK && !g_CoinInPulseStartFlag )  //有脉冲
	 {	  
		 g_coinInPulseIntervalTime = 0;  
		 g_coinInPulseWidthTime = 0;
		 g_CoinInPulseStartFlag = 1;
	 }
	 else if( COIN_INPUT_CHECK && g_CoinInPulseStartFlag )	   //脉冲结束。
	 {					 
		 g_CoinInPulseStartFlag = 0;  
		 #if  COIN_TEST_DEBUG  
		 MY_DEBUG( "Open:g_coinInPulseWidthTime==%dms\r\n",g_coinInPulseWidthTime*COIN_IN_TIMER_PERIOD );
		 #endif
	 }    
}

//脉冲宽度检测/
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
else	   //脉冲结束。
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
	   #if	COIN_TEST_DEBUG  
	   
	   MY_DEBUG( "g_coinInPulseWidthTime==%dms\r\n",FLAG.CoinPlusCount );
	   MY_DEBUG( "CoinType.CoinIn==%d\r\n",handleData.CoinType.CoinIn );
	   #endif
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
      if(restartcount++>200)
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

		
		if(handleData.CoinType.CoinScan_Up||handleData.CoinType.CoinIn_Up||handleData.CoinType.PrizePulseIn)
		{
			if(FLAG.deviceSendBackup>20*OS_TICKS_PER_SEC/2)
			{
				FLAG.deviceSendBackup=10*OS_TICKS_PER_SEC/2;//有投币的话10秒后上报，没有一分钟上报一次
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
 u8 g_coinUpTimeAcc = 0;

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
				  MY_DEBUG( "g_coinUpTimeAcc=%d\r\n", g_coinUpTimeAcc );
			 }			
			 else 
			 {
				 COIN_UP_GPIO_OUT_SET(0);//0;
				 if( g_coinUpTimeAcc*COIN_UP_TIMER_PERIOD > device.commTaskData.ioPara.plusePara[PLUSE_W0] + device.commTaskData.ioPara.plusePara[PLUSE_S0] )
				 {			
				   MY_DEBUG( "g_coinUpTimeAcc=%d\r\n", g_coinUpTimeAcc );
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
 }




















