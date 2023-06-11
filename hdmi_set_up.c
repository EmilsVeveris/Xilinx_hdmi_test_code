/*
 * HDMI_Set_Up.c
 *
 *  Created on: Jun 7, 2023
 *      Author: Admin
 */

#include "hdmi_set_up.h"

#include "xiic.h"
#include "xiic_l.h"
#include "xil_io.h"
#include "xil_types.h"

int IicLowLevelDynEeprom()
{
	u8 BytesRead;
	//u8 BytesWritten;
	u32 StatusReg;
	u8 Index;
	int Status, i;
	EepromIicAddr = IIC_SWITCH_ADDRESS;


	/*
	 * Initialize the IIC Core.
	 */
	Status = XIic_DynInit(IIC_BASE_ADDRESS);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	// Generate GPIO Reset sequence
	XIic_WriteReg(IIC_BASE_ADDRESS, 0x124, 0xffffffff);	// bit0 = 1
	for(i=0;i<100000;i++);	// delay
	XIic_WriteReg(IIC_BASE_ADDRESS, 0x124, 0xfffffffe);	// bit0 = 0
	for(i=0;i<100000;i++);	// delay
	XIic_WriteReg(IIC_BASE_ADDRESS, 0x124, 0xffffffff);	// bit0 = 1

	/*
	 * Make sure all the Fifo's are cleared and Bus is Not busy.
	 */
	while (((StatusReg = XIic_ReadReg(IIC_BASE_ADDRESS,
				XIIC_SR_REG_OFFSET)) &
				(XIIC_SR_RX_FIFO_EMPTY_MASK |
				XIIC_SR_TX_FIFO_EMPTY_MASK |
		 		XIIC_SR_BUS_BUSY_MASK)) !=
		 		(XIIC_SR_RX_FIFO_EMPTY_MASK |
				XIIC_SR_TX_FIFO_EMPTY_MASK)) {

	}

	/*
	 * Initialize the data to written and the read buffer.
	 */
	for (Index = 0; Index < PAGE_SIZE; Index++) {
		WriteBuffer[Index] = Index;
		ReadBuffer[Index] = 0;
		DataBuf[Index] = 0;
	}
	/*
	 * Write to the IIC SWITCH.
	 */
	EepromIicAddr = IIC_SWITCH_ADDRESS; // Alternate use of Write routine
	WriteBuffer[0] = 0x20;
	EepromWriteByte(0x20, DataBuf, 0);
	/*
	 * Write Initialization Sequence to ADV7511.
	 */
	EepromIicAddr = IIC_ADV7511_ADDRESS;
	for ( Index = 0; Index < NUMBER_OF_HDMI_REGS; Index++)
	{
	  EepromWriteByte(hdmi_iic[Index].addr, &hdmi_iic[Index].init, 1);
	  //for(i=0;i<100000;i++);	// delay
	  //BytesWritten =  EepromWriteByte(hdmi_iic[Index].addr, &hdmi_iic[Index].init, 1);
	  //xil_printf("\r\nBytesWritten = %d\r\n", BytesWritten);
	}

	//xil_printf("Press Any Key to CONTINUE>\r\n");
	//c = getchar();

	/*
	 * Read from the EEPROM.
	 */
	for ( Index = 0; Index < NUMBER_OF_HDMI_REGS; Index++)
	{
      BytesRead = EepromReadByte(hdmi_iic[Index].addr, &hdmi_iic[Index].data, 1);
      for(i=0;i<1000;i++);	// IIC delay
      //xil_printf("\r\nBytesRead = %d\r\n", BytesRead);

	  if (BytesRead != 1)
	  {
	    return XST_FAILURE;
	  }
           // xil_printf("ReadBuffer[%d] = %02X\r\n", Index, ReadBuffer[Index]);
	}

    /*
     * Display read buffer.
     */
	for (Index = 0; Index < NUMBER_OF_HDMI_REGS; Index++) {
         //xil_printf(" %02d) HDMI REG[0x%02X] = 0x%02X\r\n", Index, hdmi_iic[Index].addr, hdmi_iic[Index].data);
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function writes a buffer of bytes to the IIC serial EEPROM.
*
* @param	BufferPtr contains the address of the data to write.
* @param	ByteCount contains the number of bytes in the buffer to be
*		written. Note that this should not exceed the page size of the
*		EEPROM as noted by the constant PAGE_SIZE.
*
* @return	The number of bytes written, a value less than that which was
*		specified as an input indicates an error.
*
* @note		one.
*
******************************************************************************/
u8 EepromWriteByte(AddressType Address, u8 *BufferPtr,
					   u8 ByteCount)
{
	u8 SentByteCount;
	u8 WriteBuffer[sizeof(Address) + PAGE_SIZE];
	u8 Index;

	/*
	 * A temporary write buffer must be used which contains both the address
	 * and the data to be written, put the address in first based upon the
	 * size of the address for the EEPROM
	 */
	if (sizeof(AddressType) == 2) {
		WriteBuffer[0] = (u8) (Address >> 8);
		WriteBuffer[1] = (u8) (Address);
	} else if (sizeof(AddressType) == 1) {
		WriteBuffer[0] = (u8) (Address);
		EepromIicAddr |= (EEPROM_TEST_START_ADDRESS >> 8) & 0x7;
	}

	/*
	 * Put the data in the write buffer following the address.
	 */
	for (Index = 0; Index < ByteCount; Index++) {
		WriteBuffer[sizeof(Address) + Index] = BufferPtr[Index];
	}

	/*
	 * Write a page of data at the specified address to the EEPROM.
	 */
	SentByteCount = XIic_DynSend(IIC_BASE_ADDRESS, EepromIicAddr,
				WriteBuffer, sizeof(Address) + ByteCount,
				XIIC_STOP);

	/*
	 * Return the number of bytes written to the EEPROM.
	 */
	return SentByteCount - sizeof(Address);
}

/******************************************************************************
*
* This function reads a number of bytes from the IIC serial EEPROM into a
* specified buffer.
*
* @param	BufferPtr contains the address of the data buffer to be filled.
* @param	ByteCount contains the number of bytes in the buffer to be read.
*		This value is constrained by the page size of the device such
*		that up to 64K may be read in one call.
*
* @return	The number of bytes read. A value less than the specified input
*		value indicates an error.
*
* @note		None.
*
******************************************************************************/
u8 EepromReadByte(AddressType Address, u8 *BufferPtr,
 					  u8 ByteCount)
{
	u8 ReceivedByteCount;
	u8 SentByteCount;
	u16 StatusReg;

	/*
	 * Position the Read pointer to specific location in the EEPROM.
	 */
	do {
		StatusReg = XIic_ReadReg(IIC_BASE_ADDRESS, XIIC_SR_REG_OFFSET);
		if (!(StatusReg & XIIC_SR_BUS_BUSY_MASK)) {
			SentByteCount = XIic_DynSend(IIC_BASE_ADDRESS,
							EepromIicAddr,
							(u8 *) &Address,
							sizeof(Address),
    								 XIIC_REPEATED_START);
		}

	} while (SentByteCount != sizeof(Address));
		/*
		 * Receive the data.
		 */
		ReceivedByteCount = XIic_DynRecv(IIC_BASE_ADDRESS,
						 EepromIicAddr, BufferPtr,
						 ByteCount);

	/*
	 * Return the number of bytes received from the EEPROM.
	 */
	return ReceivedByteCount;
}
