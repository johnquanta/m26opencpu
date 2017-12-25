#ifndef __AT_DEVICECMD_H
#define __AT_DEVICECMD_H

#include "ql_type.h"
#include "custom_common.h"

#define AT_VERSION   "0020"

#define at_cmdLenMax 128
#define at_dataLenMax 2048

extern  u16 at_sendLen;
extern u16 at_tranLen;

extern u8 ipDataSendFlag;
#define countof(Array) (sizeof(Array)/sizeof(Array[0]))		

#define at_cmdNum   17
static u8 at_cmdLine[at_cmdLenMax];


static const u16 PLUSE_PARA_DEF[IOPARA_NUM]={
	PLUSE_WIDTH_DEF,
	PLUSE_WIDTH_DEF1,
	PLUSE_SPACE_DEF,
	PLUSE_SPACE_DEF1,
	PLUSE_IN_WIDTH_MIN,
	PLUSE_IN_WIDTH_MAX,
	PLUSE_IN_WIDTH_MIN,
	PLUSE_IN_WIDTH_MAX,
	PLUSE_IN_SPACE_DEF,
	PLUSE_IN_SPACE_DEF,	
	MAX_UP_COIN_DEF,
};

static const u16 PLUSE_PARA_RANGE[IOPARA_NUM][2]={{PLUSE_WIDTH_MIN,PLUSE_WIDTH_MAX},{PLUSE_WIDTH_MIN,PLUSE_WIDTH_MAX},
	{PLUSE_SPACE_MIN,PLUSE_SPACE_MAX},{PLUSE_SPACE_MIN,PLUSE_SPACE_MAX},
	{PLUSE_IN_WIDTH_MIN,PLUSE_IN_WIDTH_MAX},{PLUSE_IN_WIDTH_MIN,PLUSE_IN_WIDTH_MAX},{PLUSE_IN_WIDTH_MIN,PLUSE_IN_WIDTH_MAX},{PLUSE_IN_WIDTH_MIN,PLUSE_IN_WIDTH_MAX},
	{PLUSE_IN_SPACE_MIN,PLUSE_IN_SPACE_MAX},{PLUSE_IN_SPACE_MIN,PLUSE_IN_SPACE_MAX},{PLUSE_IN_SPACE_MIN,PLUSE_IN_SPACE_MAX}};


typedef struct
{
  char *at_cmdName;
  u8 at_cmdLen;
  void (*at_testCmd)(u8 id);
  void (*at_queryCmd)(u8 id);
  void (*at_setupCmd)(u8 id, char *pPara);
  void (*at_exeCmd)(u8 id);
}at_funcationType;



void device_ParaSave(Device_sut * pDevice)
;

void device_ParaLoad(Device_sut * pDevice)
;



void at_printf(const char * s);
void at_recvTask(u8 *recvBuf,u16 recvLen);
void at_cmdProcess(u8 *pAtRcvData);



void at_exeCmdNull(u8 id);


void at_setupCmdE(u8 id, char *pPara);


void at_exeCmdRst(u8 id);



void at_exeCmdGmr(u8 id)
;
void at_setupCmdIpr(u8 id, char *pPara)
;

void at_setupCmdGslp(u8 id, char *pPara)
;


void   at_cmdProcess(u8 *pAtRcvData);

void at_testCmdId(u8 id);
void at_queryCmdId(u8 id);
void at_setupCmdId(u8 id, char *pPara);
//void at_exeCmdId(u8 id);

void at_testCmdPlu(u8 id);
void at_queryCmdPlu(u8 id);
void at_setupCmdPlu(u8 id, char *pPara);
//void at_exeCmdPara(u8 id);

void at_queryCmdBill(u8 id);

void at_testCmdCoin(u8 id);
void at_setupCmdCoin(u8 id, char *pPara);
//void at_exeCmdCoin(u8 id);

void at_queryCmdCsq(u8 id);

void at_testCmdDebug(u8 id);
void at_queryCmdDebug(u8 id);
void at_setupCmdDebug(u8 id, char *pPara);

void at_exeCmdCip(u8 id);

void at_testCmdGprs(u8 id);
void at_setupCmdGprs(u8 id, char *pPara);

void at_testCmdSet(u8 id);
void at_queryCmdSet(u8 id);
void at_setupCmdSet(u8 id, char *pPara);

void at_exeCmdGret(u8 id);

void at_exeCmdGdebug(u8 id);

void at_testCmdApn(u8 id);
void at_queryCmdApn(u8 id);
void at_setupCmdApn(u8 id, char *pPara);


#define at_backOk        at_printf("\r\nOK\r\n")
#define at_backError     at_printf("\r\nERROR\r\n");
#define at_backTeError   "+CTE ERROR: %d\r\n"

#endif
