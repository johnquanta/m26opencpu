
#include "ql_stdlib.h"
#include "ql_type.h"
#include "my_stdlib.h"
#include "tool.h"



/*
  将字符串转化为16进制数
[OUT] pbDest
[IN]  pbSrc - 字符串
[IN]  nLen -  字符串的长度.

*/
void StrToHex( u8 *pbDest, char *pbSrc, u16 nLen )
{
	char h1,h2;
	u8 s1,s2;
	u16 i;   	
    u16 len;
	char SrcBuf[ nLen+1 ];
	memset( SrcBuf, 0, nLen+1 );
	if( nLen % 2)  //长度为奇数。
	{	
	  SrcBuf[0] = '0';
	  memcpy( SrcBuf+1, pbSrc, nLen );
	  len = nLen + 1;
	}
	else
	{
	  memcpy( SrcBuf, pbSrc, nLen );
	  len = nLen;
	}
	for (i=0; i<len/2; i++)
	{
	    
		h1 = SrcBuf[2*i];
		h2 = SrcBuf[2*i+1];
		
		s1 = toupper(h1) - 0x30;
		if (s1 > 9) 
		s1 -= 7;

		s2 = toupper(h2) - 0x30;
		if (s2 > 9) 
		s2 -= 7;

		pbDest[i] = s1*16 + s2;
	}
}

/*
// C prototype : void HexToStr(BYTE *pbDest, BYTE *pbSrc, int nLen)
// parameter(s): [OUT] pbDest - 存放目标字符串
//	[IN] pbSrc - 输入16进制数的起始地址
//	[IN] nLen - 16进制数的字节数
// remarks : 将16进制数转化为字符串
*/
void HexToStr(u8 *pbDest, u8 *pbSrc, int nLen)
{
	char	ddl,ddh;
	int i;

	for (i=0; i<nLen; i++)
	{
		ddh = 48 + pbSrc[i] / 16;
		ddl = 48 + pbSrc[i] % 16;
		if (ddh > 57) ddh = ddh + 7;
		if (ddl > 57) ddl = ddl + 7;
		pbDest[i*2] = ddh;
		pbDest[i*2+1] = ddl;
	}

	pbDest[nLen*2] = '\0';
}



