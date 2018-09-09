/*
 * MemoryUtils.h
 *
 *  Created on: 25/05/2018
 *      Author: Media Markt
 */

#ifndef SRC_MEMORYUTILS_H_
#define SRC_MEMORYUTILS_H_

#include "xparameters.h"
#include "xaxicdma.h"
#include "xdebug.h"
#include "xil_cache.h"

// Maximum number of saved ips
#define MAX_IP			1024

// Number of IPs stored in local cache
#define CACHE_SIZE			32

/* Source and Destination buffer for DMA transfer.
 */
static u32 SrcBuffer[CACHE_SIZE] __attribute__ ((aligned (32)));
static u32 DestBuffer[MAX_IP] __attribute__ ((aligned (32)));

static XAxiCdma AxiCdmaInstance;

#if (!defined(DEBUG))
	extern void xil_printf(const char *format, ...);
#endif

// Configure CDMA
int config_cdma();

// Reset
int resetMemory();

// Write in memory
int writeInMemory(int addr);

// Check if it is in memory
int isInMemory(int addr);

#endif /* SRC_MEMORYUTILS_H_ */
