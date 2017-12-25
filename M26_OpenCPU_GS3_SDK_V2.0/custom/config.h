/*
 * config.h
 *
 *  Created on: 2015?12?18?
 *      Author: Administrator
 */

#ifndef APP_INCLUDE_CONFIG_H_
#define APP_INCLUDE_CONFIG_H_

#define HARDWARE_GPRS_0			0x03 //generic

#define INDUSTR_GENERIC		0x01
#define INDUSTR_WASHER		0x03
#define INDUSTR_SHANYE		0x04
#define INDUSTR_COFFEEMAKER 0x05
#define INDUSTR_ICT			0x06

#define INDUSTR_LED			0x09   

#define debug11 0
#define Test_Uocion 0

#define LITTLE_BROAD 0
#define _20161216  1
#define _20170120  1
#define Cash_Recode    1
#define Mahjong    1
#define MahjongBroad    1
#define	UPSERVER	0xd6		
#define	DOWNSERVER	0xd5		
#define loction 1
#define RESEND    1
#define _20170731 1
#define DATA_MODE 0

#define KANGTAI_EN  0



#if Cash_Recode==1
#define COM_ICT_BILL	0x01
#endif

#define HARDWARE_VERSOIN_NUM	(HARDWARE_GPRS_0)
#define INDUSTR_VERSOIN_NUM		(INDUSTR_GENERIC)
//#define INDUSTR_VERSOIN_NUM      INDUSTR_ICT
#define MAIN_SOFTWARE_VERSOIN_NUM	(0x01)
#define SOFTWARE_VERSOIN_NUM		(0x0c)

#if LITTLE_BROAD==0
#if MahjongBroad==1
#define DEVICE_VERSOIN ((3<<24)|(8<<16)|(4<<8)|12)
#else
#define DEVICE_VERSOIN ((3<<24)|(3<<16)|(11<<8)|12)
#endif

#else
#define DEVICE_VERSOIN ((3<<24)|(9<<16)|(6<<8)|12)
#endif


#define SOFTWARE_UPDATA_EN	1

#define NFC_EN				0
#define NFC_POLL_TIME		180

#if (INDUSTR_VERSOIN_NUM==INDUSTR_SHANYE)
#define AT_EN	1
#else
#define AT_EN	1
#endif



#if (INDUSTR_VERSOIN_NUM==INDUSTR_SHANYE)
#define COM_START0			0xa5
#define COM_START1			0xa5
#elif (INDUSTR_VERSOIN_NUM==INDUSTR_ICT)
#define COM_START0			0x80
#define COM_START1			0x81
#elif (INDUSTR_VERSOIN_NUM==INDUSTR_111)
#define COM_START0			0xbb
#define COM_START1			0xbb
#elif (INDUSTR_VERSOIN_NUM==INDUSTR_DOLL_MACHINE)
#define COM_START0			0xaa
#define COM_START1			0xaa

#else
#define COM_START0			0xaa
#define COM_START1			0xaa

#endif
#if (INDUSTR_VERSOIN_NUM==INDUSTR_SHANYE)
#define USART_SEND_LEN		12
#define USART_RECEIVE_LEN	11
#define USART_START			0xa5
#define USART_END			0x5a

#define USART_OFF_STARTE	0
#define USART_OFF_TYPE		1
#define USART_OFF_STATE		2
#define USART_OFF_WALLET_H	3
#define USART_OFF_WALLET_L	4
#define USART_OFF_END		(USART_RECEIVE_LEN-1)
#endif

#if RESEND==1
#define COMM_VERSOIN 2017052501//2016071301//2016012701 
#else

#define COMM_VERSOIN 2017011901//2016071301//2016012701 
#endif

#define SERVER_TEST		0

#if SERVER_TEST
// #define SERVER_TEST_IP	"120.25.133.2"
// #define SERVER_TEST_PROT "9500"
#define SERVER_TEST_IP	"113.107.7.82"
#define SERVER_TEST_PROT "1000"
#endif

#define PORT_SERVER		"9136"
#define HOST_NAME		"wmq.weimaqi.net"


#define APN_NAME		"CMMTM"
#define APN_USER		0
#define APN_PASSWORD	0
#define APN_MAX_SIZE	30




#define BILL_LAST_ONE_FLASH_SIZE	0x0f


#define COM_CHECK_OFF		2
#define COM_FRAME_OFF		4
#define COM_COM_OFF			5
#define COM_DATA_OFF		6

#define COM_ENCRYPT_LEN		8//????

#define COM_LEN_MIN		10
#define COM_LOGIN		0x10
#define COM_LOGIN_LEN	34
#define COM_COIN		0x20
#define COM_COIN_LEN	10

#if	Mahjong == 1
#define COM_TIMECOIN  0x22
#define COM_TIMECOIN_LEN 10
#endif

#define COM_SET			0x30
#define COM_SET_LEN		10
#define COM_STATE		0x40
#define COM_STATE_LEN	10
#define COM_COIN_IN		0x50
#define COM_COIN_IN_LEN	18
#define COM_SET_ID		0x60
#define COM_SET_ID_LEN	10
#define COM_GET_HARD		0x70
#define COM_GET_HARD_LEN	18
#define COM_CMD		0x80
#define COM_CMD_LEN	18
#if (INDUSTR_VERSOIN_NUM==INDUSTR_SHANYE)
#define COM_USART		0x90
#define COM_USART_LEN	10
#define COM_DEPOSIT		0x92
#define COM_DEPOSIT_LEN	10
#endif
#if (SOFTWARE_UPDATA_EN==1)
#define COM_UPDATA		0xA0
#define COM_UPDATA_LEN	10
#endif
#define COM_LOGIN_RES		0x12
#define COM_LOGIN_RES_LEN	10

#if	INDUSTR_ICT==INDUSTR_VERSOIN_NUM
#define COM_ICT_BILL	0x01
#endif

#define COM_DEVICE_USART	0x82
#define COM_DEVICE_USART_LEN	18

#define COM_RECEIVE_USART	0x84
#define COM_RECEIVE_USART_LEN	10

#define COM_SEND_USART	0x84

#define COM_DEVICE_STATE	0xb0
#define   OS_TICKS_PER_SEC 1000

#define HEART_INTERVAL	(1000/OS_TICKS_PER_SEC)


#define PLUSE_WIDTH_MIN	(10/HEART_INTERVAL)
#define PLUSE_WIDTH_DEF	(50/HEART_INTERVAL)
#define PLUSE_WIDTH_DEF1	(100/HEART_INTERVAL)
#define PLUSE_WIDTH_MAX	(1000/HEART_INTERVAL)
#define PLUSE_SPACE_MIN	(10/HEART_INTERVAL)
#define PLUSE_SPACE_DEF	(100/HEART_INTERVAL)
#define PLUSE_SPACE_DEF1	(200/HEART_INTERVAL)
#define PLUSE_SPACE_MAX	(2500/HEART_INTERVAL)

#define PLUSE_IN_WIDTH_MIN	(10/HEART_INTERVAL)
#define PLUSE_IN_WIDTH_MAX	(1000/HEART_INTERVAL)
#define PLUSE_IN_SPACE_MIN	(10/HEART_INTERVAL)
#define PLUSE_IN_SPACE_DEF	(100/HEART_INTERVAL)
#define PLUSE_IN_SPACE_MAX	(2500/HEART_INTERVAL)
#define MAX_UP_COIN_DEF     (350/HEART_INTERVAL)

#define LED_CNT		(OS_TICKS_PER_SEC/5/5)


#define REMOTE_SET_OFF		20
#define IOPARA_NUM	11
#define IP_NUM		5
#define CALIBRATION_NUM		20
#define EE_TYPE	255






#endif /* APP_INCLUDE_CONFIG_H_ */
