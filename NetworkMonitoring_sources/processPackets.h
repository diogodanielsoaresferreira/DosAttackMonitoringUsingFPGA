/*
 * processPackets.h
 *
 *  Created on: 25/05/2018
 *      Author: Media Markt
 */

#ifndef SRC_PROCESSPACKETS_H_
#define SRC_PROCESSPACKETS_H_

#include "xil_printf.h"
#include "xil_io.h"
#include "xgpio_l.h"

// Maximum number of ip's stored
#define MAX_NUMBER_OF_IPS 10

// Seconds to clean the IP's stored
#define SECONDS_TO_SAVE_IPS 10

#define RED_IDX		0
#define GREEN_IDX	1
#define BLUE_IDX	2
#define BRIGHT_IDX	3

// Process new blocked packet
void newBlockedPacket(int ip);

// Process new allowed packet
void newAllowedPacket(int ip);

// Returns 1 if IP should be blocked, 0 otherwise
int shouldBlockIp(int ip);

// Timer interrupt for clean the packets on the last period
void timerInterrupt();

// Reset the blocked IP counters
void resetCounters();

#endif /* SRC_PROCESSPACKETS_H_ */
