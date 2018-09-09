/*
 * BloomFilterUtils.h
 *
 *  Created on: 21/05/2018
 *      Author: Media Markt
 */

#ifndef SRC_BLOOMFILTERUTILS_H_
#define SRC_BLOOMFILTERUTILS_H_

#include "xil_types.h"
#include "xstatus.h"
#include "xio.h"
#include "xil_printf.h"
#include "mb_interface.h"
#include "BloomFilterStream.h"

// Reset bloom filter
void resetBloomFilter();

// Check if is in the bloom filter
int isInFilter(int i);

// Write in the bloom filter
int writeInFilter(int i);

#endif /* SRC_BLOOMFILTERUTILS_H_ */
