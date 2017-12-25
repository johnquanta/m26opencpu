

#ifndef __SERVERDATAHANDLE_H
#define __SERVERDATAHANDLE_H  

/**************************************************************************/
#define COMM_VERSOIN      2017060901//2017011901    //communication version.
/*solid version. (1)hardware version. (2)industry version. (3)main software version. (4)software version.*/
#define DEVICE_VERSOIN   ((7<<24)|(1<<16)|(1<<8)|9)  
/***TEST***/
#define LOGIN_ID                "Q52051" //"7088" "Q27021" //
#define LOGIN_PASSWORD          "1234"


/*******************·þÎñÆ÷Í¨Ñ¶Êý¾Ý*******************************************************/
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
#define COM_COIN_LEN	    10
#define COM_SET_LEN	        10
#define COM_STATE_LEN	    10
#define COM_COIN_IN_LEN	    18
#define COM_SET_ID_LEN	    1
#define COM_GET_HARD_LEN    18 
#define COM_CMD_LEN	   		18
#define COM_FOTA_LEN	    10

#define COM_RSV_MAX_LEN     24   //GPRSÊÕµ½Ò»°üÊý¾Ý×î³¤µÄ³¤¶È¡
#define COM_RSV_MIN_LEN     10
#define COM_ANSWER_MAX_LEN  COM_LOGIN_LEN    //Êý¾Ý·µ»Ø×î³¤×Ö½Ú¡£
#define COM_ANSWER_MIN_LEN	10
#define COM_ENCRYPT_LEN	    8

/********************GPRS¿ØÖÆÌ¨ÃüÁî(0x80)************************************************/ 
#define  CONSOLE_CMD_MIN_LEN                         3    //¿ØÖÆÌ¨ÉèÖÃÃüÁî×îÐ¡³¤¶È£¨×Ö½Ú£©¡£

#define  CMD_COIN_UP_PULSE_OUT_WIDTH                 0  //ÉèÖÃÉÏ±ÒÂö³åÊä³ö¿í¶È¡£
#define  CMD_PRIZE_OUT_PULSE_OUT_WIDTH               1  //ÉèÖÃÍË½±Âö³åÊä³ö¿í¶È¡£

#define  CMD_COIN_UP_PULSE_OUT_INTERVAL              2  //ÉèÖÃÉÏ±ÒÂö³åÊä³ö¼ä¸ô¡£
#define  CMD_PRIZE_OUT_PULSE_OUT_INTERVAL            3  //ÉèÖÃÍË½±Âö³åÊä³ö¼ä¸ô¡£

#define  CMD_COIN_IN_PULSE_IN_MIN_WIDTH              4  //ÉèÖÃÍ¶±ÒÆ÷Âö³åÊäÈë¿í¶È×îÐ¡Öµ.
#define  CMD_COIN_IN_PULSE_IN_MAX_WIDTH              5  //ÉèÖÃÍ¶±ÒÆ÷Âö³åÊäÈë¿í¶È×î´óÖµ.

#define  CMD_PRIZE_OUT_PULSE_IN_MIN_WIDTH            6  //ÉèÖÃÍË½±Âö³åÊäÈë¿í¶È×îÐ¡Öµ.
#define  CMD_PRIZE_OUT_PULSE_IN_MAX_WIDTH            7  //ÉèÖÃÍË½±Âö³åÊäÈë¿í¶È×î´óÖµ.

#define  CMD_COIN_IN_PULSE_IN_MIN_INTERVAL           8  //ÉèÖÃÍ¶±ÒÆ÷Âö³åÊäÈë×îÐ¡¼ä¸ô.
#define  CMD_PRIZE_OUT_PULSE_IN_MIN_INTERVAL         9  //ÉèÖÃÍË½±Âö³åÊäÈë×îÐ¡¼ä¸ô.

#define  CMD_PRIZE_OUT_NONC                          20  //ÉèÖÃÍË½±ÐÅºÅ³£¿ª³£±Õ.
#define  CMD_LOCAL_SAVE                              21  //ÉèÖÃ±¾µØ±£´æ±êÖ¾..
#define  CMD_COIN_UP_NONC                            23  //ÉèÖÃÍ¶±ÒÐÅºÅ³£¿ª³£±Õ.

/**************************************************************************************/




///////////////////////////////////////////////


#endif
