



#ifndef __CUSTOM_COMMON_H__
#define __CUSTOM_COMMON_H__



#include "config.h"

#if (INDUSTR_ICT==INDUSTR_VERSOIN_NUM)
#else

#endif

#if RESEND==1
enum
{
	DEVICE_GPS=1,
	DEVICE_COIN,
	DEVICE_SYSERR,
	DEVICE_LINKERR,
	DEVICE_CCID,
};
			#endif

typedef enum{
	TASK_IO,
	TASK_COMM,
	TASK_EEPROM_WRITE
} Task_TypeDef;


typedef enum{
	MESSAGE_OUT_COIN,
	MESSAGE_IN_COIN,
	MESSAGE_IO_PARA,
	MESSAGE_IO_PARA_PLUSE,
	MESSAGE_IO_PARA_SET,
	MESSAGE_IO_PARA_RES,
	MESSAGE_LED_STATE,
	MESSAGE_ENABLE,
	MESSAGE_ORDER_SUBMIT,
	MESSAGE_BILL,
	MESSAGE_SQ,
	MESSAGE_ID_SET,
	MESSAGE_SERVER,
	MESSAGE_COM,
	MESSAGE_USART_SEND,
	MESSAGE_USART_RECEIVE,
	MESSAGE_USART_TIMEOUT,
	MESSAGE_AT,
	MESSAGE_GPRS_RESET,
	MESSAGE_IN_PLUSE_P,
	MESSAGE_GPRS_DEBUG,
	MESSAGE_APN,
	MESSAGE_EEPROM_WRITE,




//#if (INDUSTR_ICT==INDUSTR_VERSOIN_NUM)
	MESSAGE_ICT_BILL,
//#endif
	MESSAGE_EXTEND_ICT_BILL,
	MESSAGE_EXTEND_COIN_BILL,

	MESSAGE_ICT_MODE_OUT_TIMES,

	MESSAGE_DEVICE_USART,

	MESSAGE_USART_COM,// 28

	MESSAGE_USART_STATE,// 29
	MESSAGE_USART_COIN_OUT,// 30
	MESSAGE_PRIZE_OUT_BILL,
} Message_TypeDef;

typedef struct _Message {
	unsigned short int message;
	unsigned short int target;
	union {
        unsigned char data8[32];
		unsigned short int data16[16];
		unsigned int data32[8];
    } data;
} Message_sut;

typedef struct _bill {
	unsigned int countFlag;
	unsigned int AllCoin; //
	unsigned short int AllCoinIn;
	unsigned short int AllExtendIn;
	unsigned short int AllExtendVirtual;
//	unsigned int AllTimeCoin;
} Bill_sut;



//Jerry增加
typedef struct _device_info {
	unsigned char coinOutMode;//出币类型 0：脉冲 1：串口
	unsigned char deviceType;//设备类型
	unsigned char expenseFlag;//消费允许状态字  允许 0x01,禁止：0x02
	unsigned char money; // 支付金额

	unsigned char payType;//支付方式：投币 0x01. 刷卡 0x02

	unsigned char  lockStat;//充电桩锁定状态: 0-9，无锁定0xFF
	unsigned short int workingStat;//充电桩工作状态：bit0-bit9表示通道0-9是否工作
	unsigned short int  abnormalStat;//充电桩故障状态：bit0-bit9表示通道0-9是否发生故障
} Device_info_sut;
extern Device_info_sut  Device_info;


typedef enum{//串口出币，定义是网络出币，还是投币出币
			WEB,
			COIN_DEV,
	COIN_BT,
} Coin_TypeDef; // 6



typedef enum{
	DEVICE_ALL,
	DEVICE_ID,
	DEVICE_PASSWORD,
	DEVICE_SERVER_ALL,
	DEVICE_SERVER_OFF,
	DEVICE_IP,
	DEVICE_PORT,
	DEVICE_RES,
	DEVICE_SET,
	DEVICE_P_W0,
	DEVICE_P_W1,
	DEVICE_P_S0,
	DEVICE_P_S1,
	DEVICE_P_IN_W_MIN0,
	DEVICE_P_IN_W_MAX0,
	DEVICE_P_IN_W_MIN1,
	DEVICE_P_IN_W_MAX1,
	DEVICE_P_IN_S0,
	DEVICE_P_IN_S1,
	DEVICE_MAX_UP_COIN,
} DevicePara_TypeDef;

typedef enum{
	LED_STATE_NO_WIFI,
	LED_STATE_SMART,
	LED_STATE_NO_LINK,
	LED_STATE_STOP_LINK,
	LED_STATE_WORK,
	LED_STATE_READ_ERR,
	LED_STATE_GPRS_UPDATA,
		#if _20170120==1
	LED_STATE_GPRS_UPDATA_ERR,
	#endif
} LedState_TypeDef;

typedef enum{
	SERVER_OFF,
	SERVER_ALL,
	SERVER_IP,
	SERVER_PORT,
} ServerMessageTarget_TypeDef;


typedef enum{
	BIT_WIFI,
	BIT_SMART,
	BIT_LINK,
	BIT_STOP_LINK,
	BIT_LED,
	BIT_SWITCH_COIN,
	BIT_READ_ERR,
	BIT_FIRST_LOGIN,
	BIT_RESTART,
	BIT_HAVE_COIN,
	BIT_UPDATA_IP,
	BIT_DISABLE,
	BIT_ORDER_IN,
	BIT_BAN_LINK,
	BIT_MAX_UP_COIN,
	#if _20170120==1
	BIT_CLOSE_POWER,
           
	#endif
} BitFlag_TypeDef;

typedef enum{
	PLUSE_W0,
	PLUSE_W1,
	PLUSE_S0,
	PLUSE_S1,
	PLUSE_IN_W_MIN0,
	PLUSE_IN_W_MAX0,
	PLUSE_IN_W_MIN1,
	PLUSE_IN_W_MAX1,
	PLUSE_IN_S0,
	PLUSE_IN_S1,
	MAX_UP_COIN,
} PlusePara_TypeDef;

typedef enum{
	SET_PLUSE_MODE,
	SET_LOCAL_SAVE,
	SET_SOFT_PLUSE_MODE_SUPPORT,
	SET_SOFT_PLUSE_MODE,
} SetOff_TypeDef;


typedef enum{

NO_SIM_CARD,
} Gprs_errcode_TypeDef;



typedef struct _DeviceIOPara {
	unsigned short int res;
	unsigned short int set;
	unsigned short int plusePara[IOPARA_NUM];

} DeviceIOPara_sut;



typedef struct
{
	char com;
  void (*com_Cmd)(u8 *pData,u16 len);
  void (*com_CmdBk)(u8 *pPara,u16 len);
}Com_funcationType;

typedef enum{
	SERVER_LINK,
	SERVER_NO_LINK,
	SERVER_OTHER_STATE,	
} Led_state;
	extern u8 LED_STATE;
 //extern   u16 dealy_time;


/**************************************************************************/
#define DEBUG_ENABLE 1
#if DEBUG_ENABLE > 0
#define DEBUG_PORT  UART_PORT1
#define DBG_BUF_LEN   512
static char DBG_BUFFER[DBG_BUF_LEN];
#define APP_DEBUG(FORMAT,...) {\
    Ql_memset(DBG_BUFFER, 0, DBG_BUF_LEN);\
    Ql_sprintf(DBG_BUFFER,FORMAT,##__VA_ARGS__); \
    if (UART_PORT2 == (DEBUG_PORT)) \
    {\
        Ql_Debug_Trace(DBG_BUFFER);\
    } else {\
        Ql_UART_Write((Enum_SerialPort)(DEBUG_PORT), (u8*)(DBG_BUFFER), Ql_strlen((const char *)(DBG_BUFFER)));\
    }\
}
#else
#define APP_DEBUG(FORMAT,...) 
#endif

#define MY_DEBUG  APP_DEBUG
/***************************************************************************/

/*****************************************************************
* timer param
******************************************************************/
#define TCP_TIMER_ID         TIMER_ID_USER_START
#define TIMEOUT_90S_TIMER_ID TIMER_ID_USER_START + 1   //timeout  
#define LED                  TIMER_ID_USER_START + 4
#define COIN_IN_TIMER_ID     TIMER_ID_USER_START + 2
#define COIN_UP_TIMER_ID     TIMER_ID_USER_START + 3

#define TCP_TIMER_PERIOD     1000
#define LED_TIMER_PERIOD     6 
#define TIMEOUT_90S_PERIOD   30000
#define COIN_IN_TIMER_PERIOD 5    //此值小于5时，检测不到SIM卡，why??????
#define COIN_UP_TIMER_PERIOD 10

static s32 timeout_90S_monitor = FALSE;


/***************************************************************************/
#define HAL_TCP_RX_MAX  128
#define HAL_TCP_TX_MAX  128

typedef struct
{
  u8 rxBuf[HAL_TCP_RX_MAX];
  #if HAL_TCP_RX_MAX < 256
	  u8           rxHead;
      volatile u8  rxTail;
  #else
	  u16          rxHead;
	  volatile u16 rxTail;
  #endif

  u8 txBuf[HAL_TCP_TX_MAX];
  #if HAL_TCP_TX_MAX < 256
	  volatile u8  txHead;
	  u8           txTail;
  #else
	  volatile u16 txHead;
	  u16          txTail;
  #endif

}RxTxCfg_t;

/***************************************************************************/

/***********IIC*******************************************************/
//注：24C02地址最大长度为255字节。
/***IIC存储地址列表***/ 
#define   ADDR_ID             0       //ID地址，占8个字节。 
#define   ID_LEN              8

#define   ADDR_PSW            ( ADDR_ID+ID_LEN )  //密码，占8个字节。  
#define   PSW_LEN             8

#define   ADDR_IP             ( ADDR_PSW+ADDR_PSW )
#define   IP_LEN              16 //注：IP地址字符串最大长度为15，结尾加一个/0结束符，一共16个字节。

#define   ADDR_PORT           ( ADDR_IP+IP_LEN )
#define   PORT_LEN            2 //注：一个端口占2个字节。 
/*********************************************************************/

/*typedef struct _commTaskData 
{
	u8  id[ID_LEN];
	u8  password[PSW_LEN];
	u8  ip[IP_LEN];
	u16 port;
} CommTaskData_sut;*/

typedef struct _Coin 
{ 
  u16 CoinScan;  //扫码币。 
  u32 CoinScan_Up; //扫码已上的币。
  
  u16 CoinIn;    //投币器币。  
  u16 CoinIn_Up; //投币器已上的币。
  
  u16 PrizePulseIn;  //退奖脉冲输入数量。
  u16 PrizePulseIn_Up;  //退奖已上到码表的数量。

  u16 ViutueIn;  //退奖脉冲输入数量。
  u16 ViutueIn_Up;  //退奖已上到码表的数量。
} Coin_sut;



typedef struct _commTaskData {
	char id[10];//10
	char password[10];//10	
	unsigned short int serverAll;// 2
	unsigned short int serverOff;// 2
	unsigned int ip[IP_NUM]; //  20
	unsigned short int port[IP_NUM]; // 10
	DeviceIOPara_sut ioPara;  //  26 
	char apnName[APN_MAX_SIZE];// 90
	char apnUser[APN_MAX_SIZE];
	char apnPassword[APN_MAX_SIZE];

	char blank[20];
	Bill_sut bill;
} CommTaskData_sut;

typedef struct _device {
	CommTaskData_sut commTaskData;
	Bill_sut oldBill;
} Device_sut;
extern Device_sut device;


typedef struct _HandleData {
	CommTaskData_sut commTaskData;
	Coin_sut         CoinType;   
    u8 ismi[20];                //SIM卡IMSI號.
    u8 iccid[20];	            //CCID号.
	u16 HeartBeatTimeOut;//u8  GPRStransferTimeOut;////心跳超时时间( 此时间内收不到心跳包表示连接超时).
	u8 systemResetFlag;

	unsigned char frame;
	signed char rssi;

	unsigned short int coin;
	unsigned int bitFlag;
	unsigned short int reConntInterval;//单位秒
	unsigned short int conntInterval;
	unsigned int transferTime;
	unsigned int loginTime;
	unsigned int linkTime;
	unsigned int linkSuccessTime;
	unsigned int updataIPTime;

	unsigned int ismi0;
	unsigned int ismi1;


	
} HandleData_sut;

extern HandleData_sut handleData;

extern u8 restart;


typedef struct
{
 u8 coinstanormal;//	投币输入初始状态
 u8 OldCoinOutSta; //上一次输入状态
	
	 u8 coinstanormal1;//	检测扫码输入初始状态
 u8 OldCoinOutSta1; //   检测上一次扫码输入状态
	
	 u8 coinstanormal2; //按键输入初始状态
 u8 OldCoinOutSta2;   //按键上一次输入状态
	
 u8 Ready;//系统准备好了
 u16	FalutCoin[5];//异常上币发送币数到服务器
 u8 FalutCoinFlag;//异常上币发送币数到服务器标志位

	u8 iccid[10];
	
	u8 APNSave;	
	char longlat[30];
	char apnName[APN_MAX_SIZE];
	char apnUser[APN_MAX_SIZE];
	char apnPassword[APN_MAX_SIZE];
	
		#if _20161216==1				
		u16 linkfFalut[8];
		u16 simFalut;
		u16 REGFalut;		
		u16 HeartFalut;	
		u16 reconnetCount;
		u16 reconnetWWWCount;
    u8  updataIPTime;	
    u8  linkfFalutSend;		
		u8	ACKsend;
	  u16	ACKsendTimeout;
	#endif
			#if _20170120==1
		u8 Get_WebInfo;
			#endif
		
	#if USART3_EN==1
	u8 linkState;	
#endif	

			
	#if RESEND==1

u8 framebackup;
	u8 deviceSendTime;
	u32 deviceSendBackup;
	u8 WhichToSend;
	u8 Rssi;
	u8 FirstPower;
	u8 ToServerFlag;
	u8 CoinStat;
	u16 CoinPlusCount;
	#endif
 
 u8 Reconnect;
 u8 Linked;


 u32 GprsErrCode;
 u8 NoSimCardcount;
 u8 ConnetErrCount;
}S_FLAG;

extern S_FLAG FLAG;




#define HEARTBEAT_TIME_OUT_MAX     180    //GPRS心跳包超时时间


/***************************************************************************/

#endif
