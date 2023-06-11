/*
 * HDMI_Set_Up.h
 *
 *  Created on: Jun 7, 2023
 *      Author: Admin
 */

#ifndef SRC_HDMI_SET_UP_H_
#define SRC_HDMI_SET_UP_H_



#include <ctype.h>
#include <xil_types.h>
#include <stdio.h>




/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define IIC_BASE_ADDRESS	XPAR_AXI_IIC_0_BASEADDR

/*
 * The Starting address in the IIC EEPROM on which this test is performed.
 */
#define EEPROM_TEST_START_ADDRESS	0x80

/*
 * The following constant defines the address of the IIC Slave device on the
 * IIC bus. Note that since the address is only 7 bits, this constant is the
 * address divided by 2.
 * The 7 bit IIC Slave address of the IIC EEPROM on the ML300/ML310/ML403/ML410/
 * ML501/ML505/ML507/ML510 boards is 0x50. The 7 bit IIC Slave address of the IIC
 * EEPROM on the ML605/SP601/SP605 boards is 0x54.
 * Please refer the User Guide's of the respective boards for further
 * information about the IIC slave address of IIC EEPROM's.
 */
 #define EEPROM_ADDRESS		0x50

/*
 * The page size determines how much data should be written at a time.
 * The ML310/ML300 board supports a page size of 32 and 16.
 * The write function should be called with this as a maximum byte count.
 */
#define PAGE_SIZE	16

#define IIC_SLAVE_ADDRESS	1

// PCA9548 8-port IIC Switch
#define IIC_SWITCH_ADDRESS 0x74
// Connected to IIC Buses
// Bus 0
#define IIC_SI570_ADDRESS  0x5D
// Bus 1
#define IIC_FMC_HPC_ADDRESS 0x70
// Bus 2
#define IIC_FMC_LPC_ADDRESS 0x70
// Bus 3
#define IIC_EEPROM_ADDRESS 0x54
// Bus 4
#define IIC_SFP_ADDRESS 0x50
// Bus 5
#define IIC_ADV7511_ADDRESS 0x39
// Bus 6
#define IIC_DDR3_SPD_ADDRESS 0x50
#define IIC_DDR3_TEMP_ADDRESS 0x18
// Bus 7
#define IIC_SI5326_ADDRESS 0x50

#define IIC_BUS_0 0x01
#define IIC_BUS_1 0x02
#define IIC_BUS_2 0x04
#define IIC_BUS_3 0x08
#define IIC_BUS_4 0x10
#define IIC_BUS_5 0x20
#define IIC_BUS_6 0x40
#define IIC_BUS_7 0x80

/**************************** Type Definitions *******************************/

/*
 * The AddressType for ML300/ML310/ML410/ML510 boards should be u16 as the address
 * pointer in the on board EEPROM is 2 bytes.
 * The AddressType for ML403/ML501/ML505/ML507/ML605/SP601/SP605 boards should
 * be u8 as the address pointer in the on board EEPROM is 1 bytes.
 */
typedef u8 AddressType;

typedef struct {
	u8 addr;
	u8 data;
	u8 init;
} HDMI_REG;



#define NUMBER_OF_HDMI_REGS  54
HDMI_REG hdmi_iic[NUMBER_OF_HDMI_REGS] = {
	{0x01, 0x00, 0x00},
	{0x02, 0x00, 0x18},
	{0x03, 0x00, 0x00},
	{0x15, 0x00, 0x01},
	{0x16, 0x00, 0xB9},
	{0x18, 0x00, 0x46},
	{0x19, 0x00, 0x34},
	{0x1A, 0x00, 0x04},
	{0x1B, 0x00, 0xAD},
	{0x1C, 0x00, 0x00},
	{0x1D, 0x00, 0x00},
	{0x1E, 0x00, 0x1C},
	{0x1F, 0x00, 0x1B},
	{0x20, 0x00, 0x1D},
	{0x21, 0x00, 0xDC},
	{0x22, 0x00, 0x04},
	{0x23, 0x00, 0xAD},
	{0x24, 0x00, 0x1F},
	{0x25, 0x00, 0x24},
	{0x26, 0x00, 0x01},
	{0x27, 0x00, 0x35},
	{0x28, 0x00, 0x00},
	{0x29, 0x00, 0x00},
	{0x2A, 0x00, 0x04},
	{0x2B, 0x00, 0xAD},
	{0x2C, 0x00, 0x08},
	{0x2D, 0x00, 0x7C},
	{0x2E, 0x00, 0x1B},
	{0x2F, 0x00, 0x77},
	{0x40, 0x00, 0x80},
	{0x41, 0x00, 0x10},
	{0x48, 0x00, 0x08},
	{0x49, 0x00, 0xA8},
	{0x4C, 0x00, 0x00},
	{0x55, 0x00, 0x20},
	{0x56, 0x00, 0x08},
	{0x96, 0x00, 0x20},
	{0x98, 0x00, 0x03},
	{0x99, 0x00, 0x02},
	{0x9A, 0x00, 0xE0},
	{0x9C, 0x00, 0x30},
	{0x9D, 0x00, 0x61},
	{0xA2, 0x00, 0xA4},
	{0xA3, 0x00, 0xA4},
	{0xA5, 0x00, 0x44},
	{0xAB, 0x00, 0x40},
	{0xAF, 0x00, 0x06},
	{0xBA, 0x00, 0xA0},
	{0xD0, 0x00, 0x3C},
	{0xD1, 0x00, 0xFF},
	{0xDE, 0x00, 0x9C},
	{0xE0, 0x00, 0xD0},
	{0xE4, 0x00, 0x60},
	{0xF9, 0x00, 0x00}
};

u8 EepromIicAddr;		/* Variable for storing Eeprom IIC address */

int IicLowLevelDynEeprom();
u8 EepromWriteByte(AddressType Address, u8 *BufferPtr, u8 ByteCount);
u8 EepromReadByte(AddressType Address, u8 *BufferPtr, u8 ByteCount);


u8 WriteBuffer[PAGE_SIZE];	/* Write buffer for writing a page. */

u8 ReadBuffer[PAGE_SIZE];	/* Read buffer for reading a page.  */

u8 DataBuf[PAGE_SIZE];

#endif /* SRC_HDMI_SET_UP_H_ */
