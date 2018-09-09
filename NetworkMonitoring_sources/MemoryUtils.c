/*
 * MemoryUtils.c
 *
 *  Created on: 25/05/2018
 *      Author: Media Markt
 */

#include "MemoryUtils.h"

// Cache Buffer for IPs
static int cache[CACHE_SIZE];

// Size of IPs written to manage the memory
int size_memory = 0;

// Configure the CDMA
int config_cdma(){

	/* Initialize the XAxiCdma device.
	 */
	XAxiCdma_Config* CfgPtr = XAxiCdma_LookupConfig(XPAR_AXICDMA_0_DEVICE_ID);
	if (!CfgPtr) {
		return XST_FAILURE;
	}

	int Status = XAxiCdma_CfgInitialize(&AxiCdmaInstance, CfgPtr,
		CfgPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Disable interrupts, we use polling mode
	 */
	XAxiCdma_IntrDisable(&AxiCdmaInstance, XAXICDMA_XR_IRQ_ALL_MASK);


	return XST_SUCCESS;
}

// Reset memory
int resetMemory(){
	size_memory = 0;

	return 0;
}

// Transfer data from src Buffer to dst Buffer
int transferMemory(u32* srcBuffer, u32* dstBuffer, int length){

	// Wait for another DMA transfer not successfully done
	while (XAxiCdma_IsBusy(&AxiCdmaInstance)) {
		/* Wait */
	}

	int Error = XAxiCdma_GetError(&AxiCdmaInstance);
	if (Error != 0x0) {
		return XST_FAILURE;
	}


	/* Flush the SrcBuffer before the DMA transfer, in case the Data Cache
	 * is enabled
	 */
	Xil_DCacheFlushRange((UINTPTR)&srcBuffer, length*4);
#ifdef __aarch64__
	Xil_DCacheFlushRange((UINTPTR)&dstBuffer, length*4);
#endif


	/* Try to start the DMA transfer
	 */
	int retries;
	for (retries=10; retries>0; retries--) {

		int Status = XAxiCdma_SimpleTransfer(&AxiCdmaInstance, (UINTPTR)srcBuffer,
			(UINTPTR)dstBuffer, length*4, NULL, NULL);
		if (Status == XST_SUCCESS) {
			break;
		}
	}

	/* Return failure if failed to submit the transfer
	*/
	if (!retries) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

// Write address in memory
int writeInMemory(int addr){

	u32  *SrcPtr;
	SrcPtr = (u32 *)SrcBuffer;
	int status;

	if(size_memory>=MAX_IP){
		xil_printf("No available memory to write IPs!\n");
		return 1;
	}

	// Write address in cache
	cache[size_memory%CACHE_SIZE] = addr;
	// It was written in cache
	status = 0;
	// Update size memory
	size_memory++;

	// If cache is full, make transfer
	if(size_memory%CACHE_SIZE == 0){
		// Update src buffer with last block of data
		for(int i=0; i<CACHE_SIZE; i++){
			*SrcPtr = cache[i];
			SrcPtr++;
		}

		// Make transfer and update size memory
		status = transferMemory(SrcBuffer, DestBuffer+CACHE_SIZE*((size_memory-1)/CACHE_SIZE), CACHE_SIZE);
	}



	return status;
}


// Check if address is in memory
int isInMemory(int addr){

	// Check if IP in in cache
	for(int i=0; i<size_memory && i<CACHE_SIZE; i++){
		if(cache[i]==addr)
			return 1;
	}

	// If IP not in cache, check if it is in other memory blocks
	// Load block from 0 to Last-1 (last is in cache)
	int block = 0;
	while (block<CACHE_SIZE*(size_memory/CACHE_SIZE)){

		u32* DestPtr = (u32 *)DestBuffer;
		transferMemory(DestBuffer, SrcBuffer, CACHE_SIZE);

		// Wait for another transfers to be over
		while (XAxiCdma_IsBusy(&AxiCdmaInstance)) {
			/* Wait */
		}

		int Error = XAxiCdma_GetError(&AxiCdmaInstance);
		if (Error != 0x0) {
			return -XST_FAILURE;
		}

	#ifndef __aarch64__
		Xil_DCacheInvalidateRange((UINTPTR)DestPtr, 32*4);
	#endif

		// Search for address in block
		for(int i=0; i<CACHE_SIZE; i++){
			if(block+CACHE_SIZE>=CACHE_SIZE*(size_memory/CACHE_SIZE) && (i>=size_memory%CACHE_SIZE))
				break;

			if(DestPtr[i]==addr){
				return 1;
			}
		}
		block += CACHE_SIZE;
	}

	return 0;
}
