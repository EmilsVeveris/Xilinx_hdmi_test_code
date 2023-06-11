/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */



#include "menu.h"
#include "board_test_app.h"


#include "xil_printf.h"

#include <ctype.h>
#include <xil_types.h>
#include <stdio.h>
#include "xparameters.h"
#include "platform.h"
#include "xil_cache.h"
#include "xaxivdma.h"
#include "xaxivdma_i.h"


#ifndef ENABLE_ICACHE
#define ENABLE_ICACHE()		Xil_ICacheEnable()
#endif
#ifndef	ENABLE_DCACHE
#define ENABLE_DCACHE()		Xil_DCacheEnable()
#endif
#ifndef	DISABLE_ICACHE
#define DISABLE_ICACHE()	Xil_ICacheDisable()
#endif
#ifndef DISABLE_DCACHE
#define DISABLE_DCACHE()	Xil_DCacheDisable()
#endif



const uint8_t HSVpower[121] =
{0, 2, 4, 6, 8, 11, 13, 15, 17, 19, 21, 23, 25, 28, 30, 32, 34, 36, 38, 40,
42, 45, 47, 49, 51, 53, 55, 57, 59, 62, 64, 66, 68, 70, 72, 74, 76, 79, 81,
83, 85, 87, 89, 91, 93, 96, 98, 100, 102, 104, 106, 108, 110, 113, 115, 117,
119, 121, 123, 125, 127, 130, 132, 134, 136, 138, 140, 142, 144, 147, 149,
151, 153, 155, 157, 159, 161, 164, 166, 168, 170, 172, 174, 176, 178, 181,
183, 185, 187, 189, 191, 193, 195, 198, 200, 202, 204, 206, 208, 210, 212,
215, 217, 219, 221, 223, 225, 227, 229, 232, 234, 236, 238, 240, 242, 244,
246, 249, 251, 253, 255};

#define HRES 1920
#define VRES 1080
#define MEMORY_BASE XPAR_DDR4_0_BASEADDR
u32 srcBuffer = (MEMORY_BASE  + 0x1000000);

XAxiVdma InstancePtr;
XAxiVdma_DmaSetup ReadCfg;




unsigned char brot(float cx, float cy);
void fill(u8 red, u8 green, u8 blue);
void cycle_colors();
void powerHSV(int angle);


/************************** Variable Definitions *****************************/



int main()
 {

	init_platform();

	XAxiVdma_Config *Config;
	int status;

	u32 i;
	int Status;

	xil_printf("\n\r********************************************************");
	xil_printf("\n\r********************************************************");
	xil_printf("\n\r**                 KCU105 - HDMI Test                 **");
	xil_printf("\n\r********************************************************");
	xil_printf("\n\r********************************************************\r\n");

	// Setup
	Status = IicLowLevelDynEeprom();
	if (Status != XST_SUCCESS)
	{
		xil_printf("HDMI SetUp failed\r\n");
		return XST_FAILURE;
	}

	for (i = 0; i < 25000000; i++);	// delay

	Status = IicLowLevelDynEeprom();
	if (Status != XST_SUCCESS)
	{
		xil_printf("HDMI SetUp failed\r\n");
		return XST_FAILURE;
	}

	Config = XAxiVdma_LookupConfig(XPAR_AXI_VDMA_0_DEVICE_ID);

	if (!Config)
	{
		xil_printf("No video DMA found for ID %d\r\n", XPAR_AXI_VDMA_0_DEVICE_ID);
		return XST_FAILURE;
	}
	else
	{
		xil_printf("DMA found for ID %d\r\n", XPAR_AXI_VDMA_0_DEVICE_ID);
	}

	/* Initialize DMA engine */
	status = XAxiVdma_CfgInitialize(&InstancePtr, Config, Config->BaseAddress);
	if (status != XST_SUCCESS)
	{
		xil_printf("Configuration Initialization failed, status: 0x%X\r\n", status);
		return status;
	}
	else
	{
		xil_printf("Configuration Initialization sucesfull, status: 0x%X\r\n", status);
	}

	u32 stride = HRES * (Config->Mm2SStreamWidth >> 3);

	/* ************************************************** */
	/*           Setup the read channel                   */
	/*                                                    */
	/* ************************************************** */
	ReadCfg.VertSizeInput = VRES;
	ReadCfg.HoriSizeInput = stride;
	ReadCfg.Stride = stride;
	ReadCfg.FrameDelay = 0; /* This example does not test frame delay */
	ReadCfg.EnableCircularBuf = 1; /* Only 1 buffer, continuous loop */
	ReadCfg.EnableSync = 0; /* Gen-Lock */
	ReadCfg.PointNum = 0;
	ReadCfg.EnableFrameCounter = 0; /* Endless transfers */
	ReadCfg.FixedFrameStoreAddr = 0; /* We are not doing parking */

	status = XAxiVdma_DmaConfig(&InstancePtr, XAXIVDMA_READ, &ReadCfg);
	if (status != XST_SUCCESS)
	{
		xil_printf("Read channel config failed, status: 0x%X\r\n", status);
		return status;
	}
	else
	{
		xil_printf("Read channel config sucesfull, status: 0x%X\r\n", status);
	}

	// Initially populate framebuffer with complete white
	fill(0xff, 0xff, 0xff);

	/* Set the buffer addresses for transfer in the DMA engine. This is address first pixel of the framebuffer */
	status = XAxiVdma_DmaSetBufferAddr(&InstancePtr, XAXIVDMA_READ, (UINTPTR *) &srcBuffer);
	if (status != XST_SUCCESS)
	{
		xil_printf("Read channel set buffer address failed, status: 0x%X\r\n", status);
		return status;
	}
	else
	{
		xil_printf("Read channel set buffer address succesfull, status: 0x%X\r\n", status);
	}
	/************* Read channel setup done ************** */

	/* ************************************************** */
	/*  Start the DMA engine (read channel) to transfer   */
	/*                                                    */
	/* ************************************************** */

	/* Start the Read channel of DMA Engine */
	status = XAxiVdma_DmaStart(&InstancePtr, XAXIVDMA_READ);
	if (status != XST_SUCCESS)
	{
		xil_printf("Failed to start DMA engine (read channel), status: 0x%X\r\n", status);
		return status;
	}
	else
	{
		xil_printf("started DMA engine (read channel), status: 0x%X\r\n", status);
	}
	/* ************ DMA engine start done *************** */

	xil_printf("Starting HDMI monitor tests\r\n");
	cycle_colors();
	xil_printf("cycle_colors sucesfully executed\r\n");
	fill(0xff, 0xff, 0x00);
	xil_printf("fill sucesfully executed\r\n");
	mandelbrot(0, 0, 4, srcBuffer);
	xil_printf("mandelbrot sucesfully executed\r\n");

	while (1) {
		for (i = 0; i < 25000000; i++);	// delay
		xil_printf("Im in a while cycle\r\n");
	}

	cleanup_platform();
	return 0;
}




void fill(u8 red, u8 green, u8 blue) {
	int x, y;
	u32 i = 0;

	u8 *src = (u8 *) srcBuffer;

	for (y=0; y < VRES; y++) {
		for (x=0; x < HRES; x++) {
			src[i++] = blue;
			src[i++] = green;
			src[i++] = red;
		}
	}
}

void cycle_colors() {
	for (int k = 0; k < 360; k += 36)
		powerHSV(k);
}

void powerHSV(int angle)
{
  u8 red, green, blue;
  if (angle<120) {red = HSVpower[120-angle]; green = HSVpower[angle]; blue = 0;} else
  if (angle<240) {red = 0;  green = HSVpower[240-angle]; blue = HSVpower[angle-120];} else
                 {red = HSVpower[angle-240]; green = 0; blue = HSVpower[360-angle];}
  fill(red, green, blue);
}

/*
 * Function to output mandlebrot fictal, to monitor
 */
int mandelbrot()
{
	u8 *src;
	int x, y;
	float scale = 4.0 / 2;
	float cx = 0.0, cy = 0.0;

	src = (u8 *) srcBuffer;

	u32 j = 0;
	for(y = 0; y < VRES; y++)
		for(x = 0; x < HRES; x++)
		{
			unsigned char i = brot(scale * (x-HRES/2) / (HRES/2.0) + cx, scale * (y-VRES/2) / (HRES/2.0) + cy);
			src[j++] = 255 * (i&3) / 3;
			src[j++] = 255 * ((i>>2) & 7) / 7;
			src[j++] = 255 * (i>>5) / 7;
		}
	return 0;
}

unsigned char brot(float cx, float cy)
{
	int i = 0;
	float x = cx, y = cy;
	while(i < 255 && (x*x + y*y < 4.0))
	{
		float t = x;
		x = x*x - y*y + cx;
		y = 2*t*y + cy;
		i++;
	}
	return i;
}

