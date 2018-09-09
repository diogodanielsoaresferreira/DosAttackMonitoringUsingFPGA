/*
 * BloomFilterUtils.c
 *
 *  Created on: 21/05/2018
 *      Author: Media Markt
 */

#include "BloomFilterUtils.h"

void resetBloomFilter(){
	XIo_Out32(XPAR_BLOOMFILTERSTREAM_0_S00_AXI_BASEADDR+BLOOMFILTERSTREAM_S00_AXI_SLV_REG0_OFFSET, 2);
	XIo_Out32(XPAR_BLOOMFILTERSTREAM_0_S00_AXI_BASEADDR+BLOOMFILTERSTREAM_S00_AXI_SLV_REG0_OFFSET, 0);
}

int isInFilter(int i){
	unsigned int r;

	XIo_Out32(XPAR_BLOOMFILTERSTREAM_0_S00_AXI_BASEADDR+BLOOMFILTERSTREAM_S00_AXI_SLV_REG0_OFFSET, 0);
	putfsl(i, 0);
	getfsl(r, 0);
	return r;
}

int writeInFilter(int i){
	unsigned int r;
	XIo_Out32(XPAR_BLOOMFILTERSTREAM_0_S00_AXI_BASEADDR+BLOOMFILTERSTREAM_S00_AXI_SLV_REG0_OFFSET, 1);

	putfsl(i, 0);
	getfsl(r, 0);
	return r;
}
