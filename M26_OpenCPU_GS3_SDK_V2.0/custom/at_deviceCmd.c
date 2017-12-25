 

#include "at_deviceCmd.h"
#include "ql_stdlib.h"
 
#include "ql_uart.h"
 
#include "custom_common.h"
 
 at_funcationType at_fun[at_cmdNum]={
 
	 {NULL, 0, NULL, NULL, NULL, at_exeCmdNull},
	 {"AT+E", 1+2, NULL, NULL, at_setupCmdE, NULL},
	 {"AT+RST", 4+2, NULL, NULL, NULL, at_exeCmdRst},
	 {"AT+GMR", 4+2, NULL, NULL, NULL, at_exeCmdGmr},
	 {"AT+GSLP", 5+2, NULL, NULL, at_setupCmdGslp, NULL},
	 {"AT+IPR", 4+2, NULL, NULL, at_setupCmdIpr, NULL},
	 
	 //devicecmd
	 {"AT+ID", 3+2, at_testCmdId, at_queryCmdId, at_setupCmdId, NULL},
	 {"AT+PLU", 4+2, at_testCmdPlu, at_queryCmdPlu, at_setupCmdPlu, NULL},
	 {"AT+BILL", 5+2, NULL, at_queryCmdBill, NULL, NULL},
	 {"AT+COIN", 5+2, at_testCmdCoin, NULL, at_setupCmdCoin, NULL},
	 {"AT+DEBUG", 6+2, at_testCmdDebug, at_queryCmdDebug, at_setupCmdDebug, NULL},
	 {"AT+CIP", 4+2, NULL, NULL, NULL, at_exeCmdCip},
	 {"AT+GPRS", 5+2, at_testCmdGprs, NULL, at_setupCmdGprs, NULL},
	 {"AT+SET", 4+2, at_testCmdSet, at_queryCmdSet, at_setupCmdSet, NULL},
	 {"AT+GRET", 5+2, NULL, NULL, NULL, at_exeCmdGret},
	 {"AT+GDEBUG", 7+2, NULL, NULL, NULL, at_exeCmdGdebug},
	 {"AT+APN", 4+2, at_testCmdApn, at_queryCmdApn, at_setupCmdApn, NULL},
 };
 
 
  Device_sut device;
  u16 DebugLevel;
 
 void at_printf(const char * s)
 {
   Ql_UART_Write((Enum_SerialPort)(UART_PORT1), (u8*)(s), Ql_strlen((const char *)(s)));
 }


 void _IO_para_Init(DeviceIOPara_sut * pPara)
 {
	 int i=0;
	 for(i=0;i<countof(PLUSE_PARA_DEF);i++){
		 pPara->plusePara[i]=PLUSE_PARA_DEF[i];
	 }
	 pPara->set = 0;
	 pPara->res = 0;
 }

 static const char* ID = "00000000";
 static const char* PASSWARD = "12345678";

 void device_Init(Device_sut * pDevice)
 {
	 //u16 data=0;
	 u8 i=0;
	 u16 res;
	 u8 flag=0;
	 u8 wite[3]="";


    res=Ql_SecureData_Read(8,wite,3);
	 APP_DEBUG("\r\n<--res=%s-->\r\n",wite);
	 APP_DEBUG("\r\n<--flag=%d-->\r\n",flag);
	if((wite[0]=='W'&&wite[1]=='I'&&wite[2]=='R'))
    {

	APP_DEBUG("\r\n<--device ID is set-->\r\n");

	APP_DEBUG("\r\n<--pDevice->commTaskData.id=%s-->\r\n",pDevice->commTaskData.id);
	APP_DEBUG("\r\n<--pDevice->commTaskData.password=%s-->\r\n",pDevice->commTaskData.password);
	APP_DEBUG("\r\n<--pDevice->commTaskData.serverAll=%d-->\r\n",pDevice->commTaskData.serverAll);
	APP_DEBUG("\r\n<--pDevice->commTaskData.serverAll=%d-->\r\n",pDevice->commTaskData.serverOff);

	APP_DEBUG("\r\n<--pDevice->commTaskData.ip=%d %d %d %d %d -->\r\n",pDevice->commTaskData.ip[0],
		pDevice->commTaskData.ip[1],
		pDevice->commTaskData.ip[2],
		pDevice->commTaskData.ip[3],
		pDevice->commTaskData.ip[4]);

	APP_DEBUG("\r\n<--pDevice->commTaskData.port=%d %d %d %d %d -->\r\n",
		pDevice->commTaskData.port[0],
		pDevice->commTaskData.port[1],
		pDevice->commTaskData.port[2],
		pDevice->commTaskData.port[3],
		pDevice->commTaskData.port[4]);

APP_DEBUG("\r\n<--pDevice->commTaskData.res=%d-->\r\n",pDevice->commTaskData.ioPara.res);
APP_DEBUG("\r\n<--pDevice->commTaskData.set=%d-->\r\n",pDevice->commTaskData.ioPara.set);


for(i=0;i<11;i++)
	APP_DEBUG("\r\n<--pDevice->commTaskData.plusePara=%d-->\r\n",
	pDevice->commTaskData.ioPara.plusePara[i]);

	APP_DEBUG("\r\n<--APN=%s  %s   %s-->\r\n",pDevice->commTaskData.apnName,
		pDevice->commTaskData.apnUser,pDevice->commTaskData.apnPassword);

	} 
	else
	{


	   Ql_memset(pDevice,0,sizeof(Device_sut)-2*sizeof(Bill_sut));
		APP_DEBUG("\r\n<--no device ID-->\r\n");
		_IO_para_Init(&pDevice->commTaskData.ioPara);
		pDevice->commTaskData.serverAll = 0;
		pDevice->commTaskData.serverOff = 0;
		Ql_memset(pDevice->commTaskData.ip, 0, 20);
		Ql_memset(pDevice->commTaskData.port, 0, 10);
		
		Ql_memcpy(pDevice->commTaskData.id,ID,Ql_strlen(ID));
		Ql_memcpy(pDevice->commTaskData.password,PASSWARD,Ql_strlen(PASSWARD));
		
		
		for(i=0;i<countof(PLUSE_PARA_DEF);i++){
			if(pDevice->commTaskData.ioPara.plusePara[i]<PLUSE_PARA_RANGE[i][0]){
				pDevice->commTaskData.ioPara.plusePara[i]= PLUSE_PARA_RANGE[i][0];
			}else if(pDevice->commTaskData.ioPara.plusePara[i]>PLUSE_PARA_RANGE[i][1]){
				pDevice->commTaskData.ioPara.plusePara[i]= PLUSE_PARA_RANGE[i][1];
			}
		
		}


	APP_DEBUG("\r\n<--pDevice->commTaskData.id=%s-->\r\n",pDevice->commTaskData.id);
	APP_DEBUG("\r\n<--pDevice->commTaskData.password=%s-->\r\n",pDevice->commTaskData.password);
	APP_DEBUG("\r\n<--pDevice->commTaskData.serverAll=%d-->\r\n",pDevice->commTaskData.serverAll);
	APP_DEBUG("\r\n<--pDevice->commTaskData.serverAll=%d-->\r\n",pDevice->commTaskData.serverOff);

	APP_DEBUG("\r\n<--pDevice->commTaskData.ip=%d %d %d %d %d -->\r\n",pDevice->commTaskData.ip[0],
		pDevice->commTaskData.ip[1],
		pDevice->commTaskData.ip[2],
		pDevice->commTaskData.ip[3],
		pDevice->commTaskData.ip[4]);

	APP_DEBUG("\r\n<--pDevice->commTaskData.port=%d %d %d %d %d -->\r\n",
		pDevice->commTaskData.port[0],
		pDevice->commTaskData.port[1],
		pDevice->commTaskData.port[2],
		pDevice->commTaskData.port[3],
		pDevice->commTaskData.port[4]);



APP_DEBUG("\r\n<--pDevice->commTaskData.res=%d-->\r\n",pDevice->commTaskData.ioPara.res);
APP_DEBUG("\r\n<--pDevice->commTaskData.set=%d-->\r\n",pDevice->commTaskData.ioPara.set);

Ql_memset(pDevice->commTaskData.apnName,0,3*APN_MAX_SIZE);

Ql_memcpy(pDevice->commTaskData.apnName,APN_NAME,Ql_strlen(APN_NAME));


for(i=0;i<11;i++)
	APP_DEBUG("\r\n<--pDevice->commTaskData.plusePara=%d-->\r\n",
	pDevice->commTaskData.ioPara.plusePara[i]);

	APP_DEBUG("\r\n<--APN=%s  %s   %s-->\r\n",pDevice->commTaskData.apnName,
		pDevice->commTaskData.apnUser,pDevice->commTaskData.apnPassword);

	}
	 
 }

 
 void device_ParaSave(Device_sut * pDevice)
 {
   s32 res;
   res=Ql_SecureData_Store(13, (u8*)&device, sizeof(Device_sut)-2*sizeof(Bill_sut)-10); 

 APP_DEBUG("\r\device_ParaSave=%d,len=%d\r\n", res,sizeof(Device_sut)-2*sizeof(Bill_sut)-10);

 }

 void device_ParaLoad(Device_sut * pDevice)
 {//
    s32 res;
    res=Ql_SecureData_Read(13, (u8*)&device,sizeof(Device_sut)-2*sizeof(Bill_sut)-10 );
	APP_DEBUG("\r\ndevice_ParaLoad=%d  len=%d\r\n", res,sizeof(Device_sut)-2*sizeof(Bill_sut)-10);
	
 }

 static s16 at_cmdSearch(u8 cmdLen, u8 *pCmd)
  {
   u16 i;
 
   if(cmdLen == 0)
   {
	 return 0;
   }
   else if(cmdLen > 0)
   {
	 for(i=1; i<at_cmdNum; i++)
	 {
	  //APP_DEBUG("%d len %d\r\n", cmdLen, at_fun[i].at_cmdLen);
	   if(cmdLen == at_fun[i].at_cmdLen)
	   {
		 //APP_DEBUG("%s cmp %s\r\n", pCmd, at_fun[i].at_cmdName);
		 if(Ql_memcmp(pCmd, at_fun[i].at_cmdName, cmdLen) == 0) //think add cmp len first
		 {
		   return i;
		 }
	   }
	 }
   }
   return -1;
 }
 
 
 static s8 at_getCmdLen(u8 *pCmd)
 {
   u8 n,i;
 
   n = 0;
   i = 128;
 
   while(i--)
   {
	 if((*pCmd == '\r') || (*pCmd == '=') || (*pCmd == '?') || ((*pCmd >= '0')&&(*pCmd <= '9')))
	 {
	   return n;
	 }
	 else
	 {
	   pCmd++;
	   n++;
	 }
   }
   return -1;
 }
 
 
 
 
 void	at_cmdProcess(u8 *pAtRcvData)
 {
   char tempStr[32];
 
   s8 cmdId=0;
   s16 cmdLen;
   u16 i;
   //APP_DEBUG("\r\n<pAtRcvData=%s>\r\n",pAtRcvData)
 
   cmdLen = at_getCmdLen(pAtRcvData);
   //APP_DEBUG("\r\n<cmdLen=%d>\r\n",cmdLen)
   if(cmdLen != -1)
   {
	 cmdId = at_cmdSearch(cmdLen, pAtRcvData);
   }
   else 
   {
	 cmdId = -1;
   }
   if(cmdId != -1)
   {

	 pAtRcvData += cmdLen;
	 if(*pAtRcvData == '\r')
	 {

	   if(at_fun[cmdId].at_exeCmd)
	   {
		 
		 at_fun[cmdId].at_exeCmd(cmdId);
	   }
	   else
	   {
		 at_backError;
	   }
	 }
	 else if(*pAtRcvData == '?' && (pAtRcvData[1] == '\r'))
	 {
	   if(at_fun[cmdId].at_queryCmd)
	   {
		 at_fun[cmdId].at_queryCmd(cmdId);
	   }
	   else
	   {
		 at_backError;
	   }
	 }
	 else if((*pAtRcvData == '=') && (pAtRcvData[1] == '?') && (pAtRcvData[2] == '\r'))
	 {
	   if(at_fun[cmdId].at_testCmd)
	   {
		 at_fun[cmdId].at_testCmd(cmdId);
	   }
	   else
	   {
		 at_backError;
	   }
	 }
	 else if(((*pAtRcvData >= '0') && (*pAtRcvData <= '9')) || (*pAtRcvData == '='))
	 {
	   if(at_fun[cmdId].at_setupCmd)
	   {
		 at_fun[cmdId].at_setupCmd(cmdId, (char*)pAtRcvData);
	   }
	   else
	   {

		 at_backError;
	   }
	 }
	 else
	 {
	   at_backError;
	 }
   }
   else 
   {
	 at_backError;
   }
 }
 
 
 
 
 void at_exeCmdNull(u8 id)
 {
   //at_backOk;
 }
 
 
 void at_setupCmdE(u8 id, char *pPara)
 {
 
   if(*pPara == '0')
   {
	// echoFlag = FALSE;
   }
   else if(*pPara == '1')
   {
	// echoFlag = TRUE;
   }
   else
   {
	 at_backError;
	 return;
   }
   at_backOk;
 }
 
 void at_exeCmdRst(u8 id)
 {
  // at_backOk;
   //system_restart();
 }
 
 
 void at_exeCmdGmr(u8 id)
 {
	 char temp[128];
#if KANGTAI_EN == 0
   Ql_sprintf(temp,"FIRMWARE:%d.%d.%d.%d\r\n", HARDWARE_VERSOIN_NUM,INDUSTR_VERSOIN_NUM,MAIN_SOFTWARE_VERSOIN_NUM,SOFTWARE_VERSOIN_NUM);
#else
 
   Ql_sprintf(temp,"FIRMWARE:%d.%d.%d.%d\r\n", 3,5,4,12);
 
#endif
   at_printf(temp);
 
   Ql_sprintf(temp, "COMM:%d\r\n",COMM_VERSOIN);
   at_printf(temp);
 	 
   Ql_sprintf(temp, "APN:%s %s %s\r\n",device.commTaskData.apnName,device.commTaskData.apnUser,device.commTaskData.apnPassword);
   	
   at_printf(temp);

   at_backOk;
 }

 
 void at_setupCmdIpr(u8 id, char *pPara)
 {

 }
 
 void at_setupCmdGslp(u8 id, char *pPara)
 {

 }
 
 void at_testCmdId(u8 id)
 {
	 //char temp[32];
	 //Ql_sprintf(temp, "%s:(id),(password)\r\n", at_fun[id].at_cmdName);
	 //at_printf(temp);
	 //at_backOk;
 }
 void at_queryCmdId(u8 id)
 {
	 char temp[32];
	 Ql_sprintf(temp, "%s:\"%s\",\"%s\"\r\n", at_fun[id].at_cmdName,device.commTaskData.id,device.commTaskData.password);
	 at_printf(temp);

	 	//Ql_SecureData_Read(2,(u8*)&temp,20 ); 
		//APP_DEBUG("<--temp:%s -->\r\n",temp)
	 at_backOk;
 }
 
 void at_setupCmdId(u8 id, char *pPara)
 {
	 u8 buf[20];
	 u8 wirte[3]="WIR";
	 s32 ret;
	 int ii=0;
	  	 u8 flag;
	 Ql_sscanf(pPara,"=\"%[^\"]\",\"%[^\"]\"",buf,buf+10);
	 if(Ql_strlen(buf)>0 && Ql_strlen(buf)<=8 && Ql_strlen(buf+10)>0 && Ql_strlen(buf+10)<=8){

		 
		 ret= Ql_SecureData_Store(8, "000",3); 
		 
		 device_Init(&device);

		 Ql_memcpy(device.commTaskData.id,buf,10);
		 Ql_memcpy(device.commTaskData.password,buf+10,10);
		 APP_DEBUG("<--device.commTaskData.id:%s -->\r\n",buf);

	     APP_DEBUG("<--device.commTaskData.password:%s -->\r\n",buf+10);
		 
		 at_backOk;

        ret= Ql_SecureData_Store(8, wirte,3); 
		  APP_DEBUG("<--ret=%d -->\r\n",ret);
		 device_ParaSave(&device);
		 Ql_memset(buf,0,20);
		 for( ii=1;ii<6;ii++)
		 {
			ret=  Ql_SecureData_Store(ii,buf,16 ); 
			APP_DEBUG("<--ret=%d -->\r\n",ret);
			ret= Ql_SecureData_Read(2, (u8*)&device.commTaskData.bill, sizeof(device.commTaskData.bill)); 
			APP_DEBUG("<--device.commTaskData.bill:ress=%d -->\r\n",ret);
			APP_DEBUG("<--device.commTaskData.bill:%d %d %d %d %d %d -->\r\n",2,device.commTaskData.bill.countFlag,
			device.commTaskData.bill.AllCoin,
			device.commTaskData.bill.AllCoinIn,
			device.commTaskData.bill.AllExtendIn,
			device.commTaskData.bill.AllExtendVirtual);
		 }

	 }else{
		 at_backError;
	 }
 }
 
 void at_testCmdPlu(u8 id)
 {
	 char temp[32];
	 Ql_sprintf(temp, "%s:(0-9),(10-2500)\r\n", at_fun[id].at_cmdName);
	 at_printf(temp);
	 at_backOk;
 }
 void at_queryCmdPlu(u8 id)
 {
	 int i;
	 char temp[128];
	 Ql_sprintf(temp, "%s:", at_fun[id].at_cmdName);
	 for(i=0;i<IOPARA_NUM;i++){
		 Ql_sprintf(temp+Ql_strlen(temp),"%d,",device.commTaskData.ioPara.plusePara[i]*HEART_INTERVAL);
	 }
	 Ql_sprintf(temp+Ql_strlen(temp),"\r\n");
	 at_printf(temp);
	 at_backOk;
 }
 
 void at_setupCmdPlu(u8 id, char *pPara)
 {
	 int off,data;
	 Ql_sscanf(pPara,"=%d,%d",&off,&data);
	 if(off<IOPARA_NUM){
		 at_backOk;
		 device.commTaskData.ioPara.plusePara[off] = data/HEART_INTERVAL;
		// device_ParaSave(&device,(DevicePara_TypeDef)(DEVICE_P_W0+off));
		 //send_Message(TASK_COMM,MESSAGE_IO_PARA,off+DEVICE_P_W0-DEVICE_RES,(u8*)(device.commTaskData.ioPara.plusePara+off),2);
	 }else{
		 at_backError;
	 }
 }
 
 void at_queryCmdBill(u8 id)
 {
	 char temp[64];
	 Ql_sprintf(temp, "%s:%d,%d,%d,%d\r\n", at_fun[id].at_cmdName,device.commTaskData.bill.AllCoin,device.commTaskData.bill.AllCoinIn,device.commTaskData.bill.AllExtendIn,device.commTaskData.bill.AllExtendVirtual);
	 at_printf(temp);
	 at_backOk;
 }
 
 void at_testCmdCoin(u8 id)
 {
	 char temp[32];
	 Ql_sprintf(temp, "%s:(1-2),(1-200)\r\n", at_fun[id].at_cmdName);
	 at_printf(temp);
	 at_backOk;
 }
 
 void at_setupCmdCoin(u8 id, char *pPara)
 {
	 int off,data;
	 Ql_sscanf(pPara,"=%u,%u",&off,&data);
	 if(off==1 || off==2){
		 at_backOk;
		//handleData.CoinType.CoinScan = data;
		// send_Message(TASK_IO,MESSAGE_IN_COIN,off,(u8*)&data,2);
	 }else{
		 at_backError;
	 }
 }
 
 void at_testCmdDebug(u8 id)
 {
	 char temp[32];
	 Ql_sprintf(temp, "%s:(0-100)\r\n", at_fun[id].at_cmdName);
	 at_printf(temp);
	 at_backOk;
 }
 void at_queryCmdDebug(u8 id)
 {
	 char temp[32];
	 Ql_sprintf(temp, "%s:%d\r\n", at_fun[id].at_cmdName,DebugLevel);
	 at_printf(temp);
	 at_backOk;
 }
 
 
 void at_setupCmdDebug(u8 id, char *pPara)
 {
	 int data;
	 Ql_sscanf(pPara,"=%d",&data);
	 if(data<=100){
	   DebugLevel=data;
		 at_backOk;
	 }else{
		 at_backError;
	 }
 }
 
 void at_exeCmdCip(u8 id)
 {
	 at_backOk;
	 device.commTaskData.serverAll=0;
	 device.commTaskData.serverOff=0;
	 Ql_memset(device.commTaskData.ip,0,IP_NUM*4);
	 Ql_memset(device.commTaskData.port,0,IP_NUM*2);
	 //device_ParaSave(&device,DEVICE_SERVER_ALL);
	 //device_ParaSave(&device,DEVICE_SERVER_OFF);
	 //device_ParaSave(&device,DEVICE_IP);
	 //device_ParaSave(&device,DEVICE_PORT);
   //system_restart();
 }
 
 void at_testCmdGprs(u8 id)
 {
	 char temp[32];
	 Ql_sprintf(temp, "%s:(Attentio),(1000-30000)\r\n", at_fun[id].at_cmdName);
	 at_printf(temp);
	 at_backOk;
 }
 
 void at_setupCmdGprs(u8 id, char *pPara)
 {
	 char buf[20];
	 u32 time;
	 Ql_sscanf(pPara,"=\"%[^\"]\",%u",buf,&time);
	 if(time>=1000 && time<=30000){
		 at_backOk;
		// send_Message(TASK_COMM,MESSAGE_AT,time,(u8*)buf,Ql_strlen(buf)+1);
	 }else{
		 
	 }
 }
 
 void at_testCmdSet(u8 id)
 {
	 char temp[32];
	 Ql_sprintf(temp, "%s:(0-15),(0-1)\r\n", at_fun[id].at_cmdName);
	 at_printf(temp);
	 at_backOk;
 }
 
 void at_queryCmdSet(u8 id)
 {
	 char temp[32];
	 Ql_sprintf(temp, "%s:%d\r\n", at_fun[id].at_cmdName,device.commTaskData.ioPara.set);
	 at_printf(temp);
	 at_backOk;
 }
 
 void at_setupCmdSet(u8 id, char *pPara)
 {
	 int off,data;
	 Ql_sscanf(pPara,"=%u,%u",&off,&data);
	 if(off>=0 && off<16){
		 at_backOk;
		// send_Message(TASK_COMM,MESSAGE_IO_PARA_SET,off,(u8*)&data,2);
		// send_Message(TASK_IO,MESSAGE_IO_PARA_SET,off,(u8*)&data,2);
	 }else{
		 at_backError;
	 }
 }
 
 void at_exeCmdGret(u8 id)
 {
	 //at_backOk;
	// send_Message(TASK_COMM,MESSAGE_GPRS_RESET,0,0,0);
 }
 
 void at_exeCmdGdebug(u8 id)
 {
	 //at_backOk;
	 //send_Message(TASK_COMM,MESSAGE_GPRS_DEBUG,0,0,0);
	 //send_Message(TASK_IO,MESSAGE_GPRS_DEBUG,0,0,0);
 }
 
 void at_testCmdApn(u8 id)
 {
	 char temp[64];
	 Ql_sprintf(temp, "%s:(apn name),(apn user),(apn password)\r\n", at_fun[id].at_cmdName);
	 at_printf(temp);
	 at_backOk;
 }
 
 void at_queryCmdApn(u8 id)
 {
	 char temp[20+APN_MAX_SIZE*3];
	 char buf[APN_MAX_SIZE*3];
	 //apn_Load((u16*)buf);
	 //device_ParaSave(&device);
	 if(buf[0]){
		 Ql_sprintf(temp, "%s:\"%s\",\"%s\",\"%s\"\r\n", at_fun[id].at_cmdName,device.commTaskData.apnName,device.commTaskData.apnUser,device.commTaskData.apnPassword);
	 }else{
		 if(APN_USER && APN_PASSWORD)Ql_sprintf(temp, "%s:\"%s\",\"%s\",\"%s\"\r\n", at_fun[id].at_cmdName,APN_NAME,APN_USER,APN_PASSWORD);
		 else Ql_sprintf(temp, "%s:\"%s\"\r\n", at_fun[id].at_cmdName,APN_NAME);
	 }
	 at_printf(temp);
	 at_backOk;
 }
 
 void at_setupCmdApn(u8 id, char *pPara)
 {	 
	 char buf[APN_MAX_SIZE*3]="";
	 Ql_memset(buf,0,APN_MAX_SIZE*3);
	 Ql_sscanf(pPara,"=\"%[^\"]\",\"%[^\"]\",\"%[^\"]\"",buf,buf+APN_MAX_SIZE,buf+APN_MAX_SIZE*2);

	 APP_DEBUG("<--APNbuf=%s  %s  %s-->\r\n",buf,buf+APN_MAX_SIZE,buf+APN_MAX_SIZE*2);

	 if(Ql_strlen(buf)>0){
 
		
		 //apn_Save((u16*)buf);
		 
		 FLAG.APNSave=1;
		 Ql_memcpy(device.commTaskData.apnName,(u8*)buf,APN_MAX_SIZE);
		 Ql_memcpy(device.commTaskData.apnUser,(u8*)buf+APN_MAX_SIZE,APN_MAX_SIZE);
		 Ql_memcpy(device.commTaskData.apnPassword,(u8*)buf+APN_MAX_SIZE*2,APN_MAX_SIZE);
		 device_ParaSave(&device);
		  at_backOk;
		 //send_Message(TASK_COMM,MESSAGE_APN,0,(u8*)buf,APN_MAX_SIZE*3);
	 }else{
		 at_backError;
	 }
 }
 
