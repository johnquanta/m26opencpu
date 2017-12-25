/*#ifndef __TYPES_H
#define __TYPES_H

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


typedef struct _DeviceIOPara {
	unsigned short int res;
	unsigned short int set;
	unsigned short int plusePara[IOPARA_NUM];

} DeviceIOPara_sut;

typedef struct _commTaskData {
	char id[10];
	char password[10];
	unsigned short int serverAll;
	unsigned short int serverOff;
	unsigned int ip[IP_NUM];
	unsigned short int port[IP_NUM];
	DeviceIOPara_sut ioPara;
	Bill_sut bill;
} CommTaskData_sut;

typedef struct _device {
	CommTaskData_sut commTaskData;
	Bill_sut oldBill;
} Device_sut;

typedef struct _HandleData {
	CommTaskData_sut commTaskData;
	
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
	unsigned short int inPluseP[4];
	char apnName[APN_MAX_SIZE];
	char apnUser[APN_MAX_SIZE];
	char apnPassword[APN_MAX_SIZE];

	
} HandleData_sut;


typedef struct
{
	char com;
  void (*com_Cmd)(u8 *pData,u16 len);
  void (*com_CmdBk)(u8 *pPara,u16 len);
}Com_funcationType;



#endif*/
