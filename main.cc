// set logfile [open "D:\\Vivado_HLS_Tutorial\\2D_convolution_example\\log.txt" "w"]
// puts $logfile [mrd 0x1300000 76800]
// close $logfile

#include <stdio.h>
#include "xaxidma.h"
#include "ab_in_txt.h"
#include "xdoimgproc.h"
#include "AxiTimerHelper.h"

#define SIZE_ARR (320*240)

#define KERNEL_DIM 3

//Memory used by DMA, define DMA address
#define MEM_BASE_ADDR 0x01000000
#define TX_BUFFER_BASE (MEM_BASE_ADDR + 0x00100000)
#define RX_BUFFER_BASE (MEM_BASE_ADDR + 0x00300000)

unsigned char *m_dma_buffer_TX = (unsigned char*)TX_BUFFER_BASE;
unsigned char *m_dma_buffer_RX = (unsigned char*)RX_BUFFER_BASE;

unsigned char imgIn_HW[SIZE_ARR]; //Array which used to copy the image

XAxiDma axiDma;

int initDMA()
{
	XAxiDma_Config *CfgPtr;
	CfgPtr = XAxiDma_LookupConfig(XPAR_AXI_DMA_0_DEVICE_ID);
	XAxiDma_CfgInitialize(&axiDma, CfgPtr);

	//Disable interrupts
	XAxiDma_IntrDisable(&axiDma, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DEVICE_TO_DMA);
	XAxiDma_IntrDisable(&axiDma, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DMA_TO_DEVICE);

	return XST_SUCCESS;
}

XDoimgproc doImgProc;

int initDoImgProc() //Init IP core
{
	int status;

	XDoimgproc_Config *doImgProc_cfg;
	doImgProc_cfg = XDoimgproc_LookupConfig(XPAR_DOIMGPROC_0_DEVICE_ID);

	if (!doImgProc_cfg)
	{
		printf("Error loading config for doHist_cfg\n");
	}

	status = XDoimgproc_CfgInitialize(&doImgProc, doImgProc_cfg);

	if (status != XST_SUCCESS)
	{
		printf("Error initializing for doHist\n");
	}

	return status;
}


char kernel[KERNEL_DIM*KERNEL_DIM] =
{
	0,0,0,
	0,1,0,
	0,0,0,
};

int main()
{
	initDMA();
	initDoImgProc();

	printf("Doing convulution on HW\n");

	//Populate data , get image
	for (int idx = 0; idx < SIZE_ARR; idx++)
	{
		imgIn_HW[idx] = image_ab[idx];
	}

	AxiTimerHelper axiTimer;

	printf("Starting....HW\n");

	XDoimgproc_Write_kernel_Bytes(&doImgProc, 0, kernel, 9);
	printf("Kernel total bytes: %ld Bitwidth:%ld Base: 0x%ld\n", XDoimgproc_Get_kernel_TotalBytes(&doImgProc),XDoimgproc_Get_kernel_BitWidth(&doImgProc), XDoimgproc_Get_kernel_BaseAddress(&doImgProc));
	XDoimgproc_Set_operation(&doImgProc, 0);
	XDoimgproc_Start(&doImgProc); // Send start

	//DO the DMA transfer to push and get our image
	axiTimer.startTimer();
	Xil_DCacheFlushRange((u32)imgIn_HW, SIZE_ARR*sizeof(unsigned char));
	Xil_DCacheFlushRange((u32)m_dma_buffer_RX,SIZE_ARR*sizeof(unsigned char));

	XAxiDma_SimpleTransfer(&axiDma, (u32)imgIn_HW, SIZE_ARR*sizeof(unsigned char), XAXIDMA_DMA_TO_DEVICE);
	XAxiDma_SimpleTransfer(&axiDma, (u32)m_dma_buffer_RX, SIZE_ARR*sizeof(unsigned char), XAXIDMA_DEVICE_TO_DMA);


	//Wait transfer to finish
	while(XAxiDma_Busy(&axiDma, XAXIDMA_DMA_TO_DEVICE));
	while(XAxiDma_Busy(&axiDma, XAXIDMA_DEVICE_TO_DMA));

	for (int idx = 0; idx < 1000; idx++)
			{
				printf("Input No.%d:%d\n", idx, imgIn_HW[idx]);
			}

	for (int idx = 0; idx < 1000; idx++)
				{
					printf("TxInput No.%d:%d\n", idx, m_dma_buffer_TX[idx]);
				}


	for (int idx = 0; idx < 1000; idx++)
		{
			printf("Rx Output No.%d:%d\n", idx, m_dma_buffer_RX[idx]);
		}

	//Incalidate the cache to avoid reading garbage
	Xil_DCacheInvalidateRange((u32)m_dma_buffer_RX, SIZE_ARR*sizeof(unsigned char));
	axiTimer.stopTimer();

	double HW_elapsed = axiTimer.getElapsedTimerInSeconds();
	//double time_in_ns = HW_elapsed*1000000000;
	printf("HW execution time: %f sec\n", HW_elapsed);

	return 0;
}
