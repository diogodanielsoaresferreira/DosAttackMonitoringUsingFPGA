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

#include <stdio.h>

#include "xparameters.h"

#include "netif/xadapter.h"
#include "BloomFilterUtils.h"
#include "processPackets.h"
#include "xil_exception.h"
#include "xparameters.h"
#include "xintc_l.h"
#include "xgpio_l.h"
#include "MemoryUtils.h"

#include "platform.h"
#include "platform_config.h"
#if defined (__arm__) || defined(__aarch64__)
#include "xil_printf.h"
#endif

#include "lwip/tcp.h"
#include "xil_cache.h"

#if LWIP_DHCP==1
#include "lwip/dhcp.h"
#endif

#include "xil_io.h"


/* defined by each RAW mode application */
void print_app_header();
int start_application();
int transfer_data();
void tcp_fasttmr(void);
void tcp_slowtmr(void);

/* missing declaration in lwIP */
void lwip_init();

#if LWIP_DHCP==1
extern volatile int dhcp_timoutcntr;
err_t dhcp_start(struct netif *netif);
#endif

extern volatile int TcpFastTmrFlag;
extern volatile int TcpSlowTmrFlag;
static struct netif server_netif;
struct netif *echo_netif;

static int port = 7;

void
print_ip(char *msg, struct ip_addr *ip)
{
	print(msg);
	xil_printf("%d.%d.%d.%d\n\r", ip4_addr1(ip), ip4_addr2(ip),
			ip4_addr3(ip), ip4_addr4(ip));
}

void
print_ip_settings(struct ip_addr *ip, struct ip_addr *mask, struct ip_addr *gw)
{

	print_ip("Board IP: ", ip);
	print_ip("Netmask : ", mask);
	print_ip("Gateway : ", gw);
}

#if defined (__arm__) && !defined (ARMR5)
#if XPAR_GIGE_PCS_PMA_SGMII_CORE_PRESENT == 1 || XPAR_GIGE_PCS_PMA_1000BASEX_CORE_PRESENT == 1
int ProgramSi5324(void);
int ProgramSfpPhy(void);
#endif
#endif

#ifdef XPS_BOARD_ZCU102
#ifdef XPAR_XIICPS_0_DEVICE_ID
int IicPhyReset(void);
#endif
#endif


int config_ethernet(){
	struct ip_addr ipaddr, netmask, gw;

	/* the mac address of the board. this should be unique per board */
	unsigned char mac_ethernet_address[] =
	{ 0x00, 0x0a, 0x35, 0x00, 0x01, 0x02 };

	echo_netif = &server_netif;
	#if defined (__arm__) && !defined (ARMR5)
	#if XPAR_GIGE_PCS_PMA_SGMII_CORE_PRESENT == 1 || XPAR_GIGE_PCS_PMA_1000BASEX_CORE_PRESENT == 1
		ProgramSi5324();
		ProgramSfpPhy();
	#endif
	#endif

	/* Define this board specific macro in order perform PHY reset on ZCU102 */
	#ifdef XPS_BOARD_ZCU102
		IicPhyReset();
	#endif

	init_platform();

	#if LWIP_DHCP==1
	    ipaddr.addr = 0;
		gw.addr = 0;
		netmask.addr = 0;
	#else
		/* initialize IP addresses to be used */
		IP4_ADDR(&ip_addr, 169, 254,  66, 118);
		IP4_ADDR(&netmask, 255, 255,   0,  0);
		IP4_ADDR(&gw,      169, 254,   66,  117);
	#endif

	lwip_init();

	/* Add network interface to the netif_list, and set it as default */
	if (!xemac_add(echo_netif, &ipaddr, &netmask,
				&gw, mac_ethernet_address,
				PLATFORM_EMAC_BASEADDR)) {
		xil_printf("Error adding N/W interface\n\r");
		return -1;
	}
	netif_set_default(echo_netif);

	/* now enable interrupts */
	platform_enable_interrupts();

	/* specify that the network if is up */
	netif_set_up(echo_netif);

	#if (LWIP_DHCP==1)
		/* Create a new DHCP client for this interface.
		 * Note: you must call dhcp_fine_tmr() and dhcp_coarse_tmr() at
		 * the predefined regular intervals after starting the client.
		 */
		dhcp_start(echo_netif);
		dhcp_timoutcntr = 1;

		while(((echo_netif->ip_addr.addr) == 0) && (dhcp_timoutcntr > 0))
			xemacif_input(echo_netif);

		if (dhcp_timoutcntr <= 0) {
			if ((echo_netif->ip_addr.addr) == 0) {
				xil_printf("Configuring default IP of 169.254.66.118\r\n");
				IP4_ADDR(&(echo_netif->ip_addr),  169, 254,  66, 118);
				IP4_ADDR(&(echo_netif->netmask), 255, 255,   0,  0);
				IP4_ADDR(&(echo_netif->gw),      169, 254,   66,  117);
			}
		}

		ipaddr.addr = echo_netif->ip_addr.addr;
		gw.addr = echo_netif->gw.addr;
		netmask.addr = echo_netif->netmask.addr;
	#endif

	print_ip_settings(&ipaddr, &netmask, &gw);

	return 0;
}

// Init the RGB led 0
void config_rgb1(){
	Xil_Out32(XPAR_RGB_LED_CONTROLLER_0_S00_AXI_BASEADDR + (RED_IDX 	 * 4), 0);
	Xil_Out32(XPAR_RGB_LED_CONTROLLER_0_S00_AXI_BASEADDR + (GREEN_IDX  * 4), 255);
	Xil_Out32(XPAR_RGB_LED_CONTROLLER_0_S00_AXI_BASEADDR + (BLUE_IDX 	 * 4), 0);
	Xil_Out32(XPAR_RGB_LED_CONTROLLER_0_S00_AXI_BASEADDR + (BRIGHT_IDX * 4), 500);
}

// Init the RGB led 1
void config_rgb2(){
	Xil_Out32(XPAR_RGB_LED_CONTROLLER_1_S00_AXI_BASEADDR + (RED_IDX 	 * 4), 0);
	Xil_Out32(XPAR_RGB_LED_CONTROLLER_1_S00_AXI_BASEADDR + (GREEN_IDX  * 4), 255);
	Xil_Out32(XPAR_RGB_LED_CONTROLLER_1_S00_AXI_BASEADDR + (BLUE_IDX 	 * 4), 0);
	Xil_Out32(XPAR_RGB_LED_CONTROLLER_1_S00_AXI_BASEADDR + (BRIGHT_IDX * 4), 500);
}

// Init all displays with '0'
void config_displays(){
	char digitEnable[] = {1, 1, 1, 1, 1, 1, 1, 1};
	static char digit[] = {0, 0, 0, 0, 0, 0, 0, 0};

	Xil_Out32(XPAR_DISPLAYCONTROLLER_0_S00_AXI_BASEADDR,
				(digitEnable[0] & 1) | (digitEnable[1] & 1)<<1 | (digitEnable[2] & 1)<<2 | (digitEnable[3] & 1)<<3 |
				(digitEnable[4] & 1)<<4 | (digitEnable[5] & 1)<<5 | (digitEnable[6] & 1)<<6 | (digitEnable[7] & 1)<<7);

	Xil_Out8(XPAR_DISPLAYCONTROLLER_0_S00_AXI_BASEADDR+8,
			(digit[0] & 15) | (digit[1] & 15)<<4);

		Xil_Out8(XPAR_DISPLAYCONTROLLER_0_S00_AXI_BASEADDR+9,
			(digit[2] & 15) | (digit[3] & 15)<<4);

		Xil_Out8(XPAR_DISPLAYCONTROLLER_0_S00_AXI_BASEADDR+10,
			(digit[4] & 15) | (digit[5] & 15)<<4);

		Xil_Out8(XPAR_DISPLAYCONTROLLER_0_S00_AXI_BASEADDR+11,
			(digit[6] & 15) | (digit[7] & 15)<<4);
}

// Setup timer and button interrupts
void setupInterrupts(){

	/*
	 * Connect a callback handler that will be called when an interrupt for the timer occurs,
	 * to perform the specific interrupt processing for the timer.
	 */
	XIntc_RegisterHandler(XPAR_INTC_0_BASEADDR, XPAR_MICROBLAZE_0_AXI_INTC_FIT_TIMER_0_INTERRUPT_INTR,
			  (XInterruptHandler)timerInterrupt, (void *)0);

	/*
	 * Enable interrupts at the push buttons GPIO
	 */
	XGpio_WriteReg(XPAR_GPIO_0_BASEADDR, XGPIO_IER_OFFSET, XGPIO_IR_CH1_MASK);
	XGpio_WriteReg(XPAR_GPIO_0_BASEADDR, XGPIO_GIE_OFFSET, XGPIO_GIE_GINTR_ENABLE_MASK);

	/*
	 * Connect a callback handler that will be called when an interrupt for the push buttons
	 * GPIO occurs, to perform the specific interrupt processing for the input port.
	 */
	XIntc_RegisterHandler(XPAR_INTC_0_BASEADDR, XPAR_MICROBLAZE_0_AXI_INTC_AXI_GPIO_0_IP2INTC_IRPT_INTR,
		  (XInterruptHandler)resetCounters, (void *)0);


	/*
	 * Enable interrupts for all devices that cause interrupts, and enable
	 * the INTC master enable bit.
	 */
	XIntc_EnableIntr(XPAR_INTC_0_BASEADDR, XPAR_AXI_ETHERNETLITE_0_IP2INTC_IRPT_MASK | XPAR_AXI_TIMER_0_INTERRUPT_MASK | XPAR_FIT_TIMER_0_INTERRUPT_MASK | XPAR_AXI_GPIO_0_IP2INTC_IRPT_MASK);

}

int main()
{
	resetBloomFilter();
	config_rgb1();
	config_rgb2();
	config_displays();
	config_cdma();
	config_ethernet();
	setupInterrupts();

	echo_netif = &server_netif;

	/* start the application */
	start_application(port);


	/* receive and process packets */
	while (1) {

		if (TcpFastTmrFlag) {
			tcp_fasttmr();
			TcpFastTmrFlag = 0;
		}
		if (TcpSlowTmrFlag) {
			tcp_slowtmr();
			TcpSlowTmrFlag = 0;
		}
		xemacif_input(echo_netif);
	}

	/* never reached */
	cleanup_platform();

	return 0;
}
