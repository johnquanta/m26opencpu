

#ifndef __COIN_PRIZE_H__
#define __COIN_PRIZE_H__

/**********************************************************************/
#define  NONC_CHECK_TIME   1000   //常开常闭自动检测的时间，单位ms。
#define  COIN_MAX_VALUE    300    //单笔最大币数。

/**********************************************************************/
#define COIN_UP_PIN    PINNAME_NETLIGHT//PINNAME_RTS
#define COIN_IN_PIN    PINNAME_RI

#define COIN_PRIZE_PIN    PINNAME_DCD

//#define NETLIGHT_PIN   PINNAME_NETLIGHT

#define COIN_INPUT_CHECK  Ql_GPIO_GetLevel( COIN_IN_PIN )   
#define COIN_UP_GPIO_OUT_SET(x)  Ql_GPIO_SetLevel( COIN_UP_PIN, x )

#define COIN_PRIZE_CHECK  Ql_GPIO_GetLevel( COIN_PRIZE_PIN )    




/***常开常闭***********************************************************/
typedef enum { OPEN = 0, CLOSE = 1 } PLUSE_MODE;

/**********************************************************************/
typedef struct
{
  //注：POW:pulse out width(脉冲输出宽度)。 POI:pulse out inerval(脉冲输出间隔)。 
  //    PIWmin:pulse int width minimum(脉冲输入宽度最小值)。PIWmax:pulse in width maximum(脉冲输入最大值)。
  //    PIImin:pulse in minimum( 脉冲输入最小间隔 )
  u8 CoinUp_POW;     //上币脉冲输出宽度.
  u8 PrizeOut_POW;   //退奖脉冲输出宽度 
    
  u8 CoinUp_POI;      //上币脉冲输出间隔。
  u8 PrizeOut_POI;    //退奖脉冲输出间隔。
  
  u8 CoinIn_PIWmin;    //投币器脉冲输入宽度最小值.
  u8 CoinIn_PIWmax;    //投币器脉冲输入宽度最大值.
  
  u8 PrizeOut_PIWmin;  //退奖脉冲输入宽度最小值.
  u16 PrizeOut_PIWmax;  //退奖脉冲输入宽度最大值.
  
  u8 CoinIn_PIImin;     //投币器脉冲输入最小间隔.
  u8 PrizeOut_PIImin;   //退奖脉冲输入最小间隔.
  
  u8 PrizeOut_NONC;  //退奖信号常开常闭.
  u8 LocalSaveFlag;  //本地保存标志.
  u8 CoinUp_NONC;    //投币信号常开常闭.  
  
}IOdata_sut;  //注：定义此结构体的变量的时候初始化。

/**********************************************************************/
void gpioInit(void);
void Callback_CoinInPulseCheck( u32 timerId, void* param );
void Callback_CoinUpPulseOut( u32 timerId, void* param );
void bill_Write(u32 wirteAddr,Bill_sut *pNewBill, Bill_sut *pOldBill);
void bill_Read(void);


#endif



