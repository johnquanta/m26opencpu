/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Quectel Co., Ltd. 2013
*
*****************************************************************************/
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   example_iic.c
 *
 * Project:
 * --------
 *   OpenCPU
 *
 * Description:
 * ------------
 *   This example demonstrates how to use iic function with APIs in OpenCPU.
 *   Input the specified command through any uart port and the result will be 
 *   output through the debug port.
 *   you can use any of two pins for the simultion IIC.And we have a IIC Controller
 *   interface ,PINNAME_GPIO8 for IIC SCL pin and PINNAME_GPIO9 for IIC SDA pin.
 *
 * Usage:
 * ------
 *   Compile & Run:
 *
 *     Set "C_PREDEF=-D __EXAMPLE_IIC__" in gcc_makefile file. And compile the 
 *     app using "make clean/new".
 *     Download image bin to module to run.
 * 
 *   Operation:
 *            
 *     If input "Ql_IIC_Init=", that will initialize the IIC channel.
 *     If input "Ql_IIC_Config=", that will configure the IIC parameters.
 *     If input "Ql_IIC_Write=", that will write bytes to slave equipment through IIC interface.
 *     If input "Ql_IIC_Read=", that will read bytes from slave equipment through IIC interface.
 *     If input "Ql_IIC_Write_Read=", that will read and write bytes through IIC interface.
 *     If input "Ql_IIC_Uninit=", that will release the IIC pins.
 *
 * Author:
 * -------
 * -------
 *
 *============================================================================
 *             HISTORY
 *----------------------------------------------------------------------------
 * 
 ****************************************************************************/

#include "ql_type.h"
#include "ql_stdlib.h"
#include "ql_trace.h"
#include "ql_gpio.h"
#include "ql_uart.h"
#include "ql_iic.h"
#include "ql_error.h"

#include "Ee2prom.h"

void eeprom_Init(void)
{
	Ql_IIC_Init(0,PINNAME_CTS,PINNAME_RTS,0);
	Ql_IIC_Config(0,TRUE, 0xa0, 0);

}


unsigned char I2CWriteByte(unsigned int mem_addr,unsigned char*DDATAp,unsigned int count)
{
  Ql_IIC_Write(0, 0xa0+mem_addr, DDATAp, count);

}



