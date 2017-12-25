

#ifndef __FOTA_BY_HTTP_H__
#define __FOTA_BY_HTTP_H__

/****************************************************************************
* Define local param
****************************************************************************/
#define APN      "CMMTM"
#define USERID   ""
#define PASSWD   ""

#define UPGRADE_BIN_NAME   "M26_OpenCPU.bin"
#define UPGRADING_YES      "Yes"

typedef struct
{
  u32 UpgradeFWversion;
  u32 UpgradeIP;
  u16 UpgradePort;
  u16 UpgradeToken;
  u8  Upgrading[4];
}foatInfo_t;


void Fota_DataHandle( u8 *pDataIn );
void Fota_Pro( u8 *BoardID );

#endif



