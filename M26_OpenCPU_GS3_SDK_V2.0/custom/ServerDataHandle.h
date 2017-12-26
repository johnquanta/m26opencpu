

#ifndef __SERVERDATAHANDLE_H
#define __SERVERDATAHANDLE_H  

/**************************************************************************/
#ifdef COMM_VERSOIN //20171226//
#undef COMM_VERSOIN  //20171226//
#endif

#define COMM_VERSOIN      2017060901//2017011901    //communication version.


#ifdef DEVICE_VERSOIN //20171226//
#undef DEVICE_VERSOIN  //20171226//
#endif

/*solid version. (1)hardware version. (2)industry version. (3)main software version. (4)software version.*/
#define DEVICE_VERSOIN   ((7<<24)|(1<<16)|(1<<8)|1)  
/***TEST***/
#define LOGIN_ID                "Q52051" //"7088" "Q27021" //
#define LOGIN_PASSWORD          "1234"


/*******************服务器通讯数据*******************************************************/
#define COM_HEAD_OFF        0
#define COM_LEN_OFF		    1
#define COM_CHECK_OFF		2
#define COM_FRAME_OFF		4
#define COM_CMD_OFF		    5
#define COM_DATA_OFF		6

#define COM_DATA_HEAD   	0xD5
#define COM_DATA_HEAD2   	0xD6
#define COM_LOGIN			0x10
#define COM_LOGIN_RES		0x12
#define COM_COIN_SCAN		0x20
#define COM_SET				0x30
#define COM_STATE			0x40
#define COM_COIN_IN			0x50
#define COM_SET_ID			0x60
#define COM_GET_HARD		0x70
#define COM_CMD		        0x80
#define COM_FOTA            0xA0

#define COM_LOGIN_LEN	    34
#define COM_LOGIN_RES_LEN	10
#ifndef COM_COIN_LEN//20171226//
#define COM_COIN_LEN	    10
#endif
#define COM_SET_LEN	        10
#define COM_STATE_LEN	    10
#define COM_COIN_IN_LEN	    18
#define COM_SET_ID_LEN	    1
#define COM_GET_HARD_LEN    18 
#define COM_CMD_LEN	   		18
#define COM_FOTA_LEN	    10

#define COM_RSV_MAX_LEN     24   //GPRS收到一包数据最长的长度?
#define COM_RSV_MIN_LEN     10
#define COM_ANSWER_MAX_LEN  COM_LOGIN_LEN    //数据返回最长字节。
#define COM_ANSWER_MIN_LEN	10
#define COM_ENCRYPT_LEN	    8

/********************GPRS控制台命令(0x80)************************************************/ 
#define  CONSOLE_CMD_MIN_LEN                         3    //控制台设置命令最小长度（字节）。

#define  CMD_COIN_UP_PULSE_OUT_WIDTH                 0  //设置上币脉冲输出宽度。
#define  CMD_PRIZE_OUT_PULSE_OUT_WIDTH               1  //设置退奖脉冲输出宽度。

#define  CMD_COIN_UP_PULSE_OUT_INTERVAL              2  //设置上币脉冲输出间隔。
#define  CMD_PRIZE_OUT_PULSE_OUT_INTERVAL            3  //设置退奖脉冲输出间隔。

#define  CMD_COIN_IN_PULSE_IN_MIN_WIDTH              4  //设置投币器脉冲输入宽度最小值.
#define  CMD_COIN_IN_PULSE_IN_MAX_WIDTH              5  //设置投币器脉冲输入宽度最大值.

#define  CMD_PRIZE_OUT_PULSE_IN_MIN_WIDTH            6  //设置退奖脉冲输入宽度最小值.
#define  CMD_PRIZE_OUT_PULSE_IN_MAX_WIDTH            7  //设置退奖脉冲输入宽度最大值.

#define  CMD_COIN_IN_PULSE_IN_MIN_INTERVAL           8  //设置投币器脉冲输入最小间隔.
#define  CMD_PRIZE_OUT_PULSE_IN_MIN_INTERVAL         9  //设置退奖脉冲输入最小间隔.

#define  CMD_PRIZE_OUT_NONC                          20  //设置退奖信号常开常闭.
#define  CMD_LOCAL_SAVE                              21  //设置本地保存标志..
#define  CMD_COIN_UP_NONC                            23  //设置投币信号常开常闭.

/**************************************************************************************/




///////////////////////////////////////////////


#endif
