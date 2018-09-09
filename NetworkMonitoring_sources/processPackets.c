/*
 * processPackets.c
 *
 *  Created on: 25/05/2018
 *      Author: Media Markt
 */

#include "processPackets.h"

// Number of blocked packets
int NUM_BLOCKED = 0;

// Total number of packets
int NUM_PACKETS = 0;

// 1 if the last received packet was blocked, 0 otherwise
int LAST_PACKET_BLOCKED = 0;

// Save the last ip's of packets
int last_ips[MAX_NUMBER_OF_IPS];

// Number of the ips saved
int number_of_saved_ips = 0;

// Offset of the index in the array to save the next IP
int current_offset = 0;

// Current second marked with the timer
int current_second = 0;


// Updates the RGB LED with the overall count of blocked packets.
void update_rgb1(){

	// DO NOT USE floating point calculations. The processor is not enabled with the floating point unit.
	int percentage = 255*(100*NUM_BLOCKED)/NUM_PACKETS;

	Xil_Out32(XPAR_RGB_LED_CONTROLLER_0_S00_AXI_BASEADDR + (RED_IDX * 4), 0);
	if (NUM_PACKETS==0){
		Xil_Out32(XPAR_RGB_LED_CONTROLLER_0_S00_AXI_BASEADDR + (GREEN_IDX * 4), 255);
		Xil_Out32(XPAR_RGB_LED_CONTROLLER_0_S00_AXI_BASEADDR + (BLUE_IDX * 4), 0);
	}
	else{
		Xil_Out32(XPAR_RGB_LED_CONTROLLER_0_S00_AXI_BASEADDR + (GREEN_IDX * 4), 255-(int)(percentage/100));
		Xil_Out32(XPAR_RGB_LED_CONTROLLER_0_S00_AXI_BASEADDR + (BLUE_IDX * 4), (int)(percentage/100));
	}
	Xil_Out32(XPAR_RGB_LED_CONTROLLER_0_S00_AXI_BASEADDR + (BRIGHT_IDX * 4), 500);
}

// Updates the RGB led with the last packet (red if blocked, green if not blocked).
void update_rgb2(){
	if (LAST_PACKET_BLOCKED==0){
		Xil_Out32(XPAR_RGB_LED_CONTROLLER_1_S00_AXI_BASEADDR + (RED_IDX 	 * 4), 0);
		Xil_Out32(XPAR_RGB_LED_CONTROLLER_1_S00_AXI_BASEADDR + (GREEN_IDX  * 4), 255);
		Xil_Out32(XPAR_RGB_LED_CONTROLLER_1_S00_AXI_BASEADDR + (BLUE_IDX 	 * 4), 0);
		Xil_Out32(XPAR_RGB_LED_CONTROLLER_1_S00_AXI_BASEADDR + (BRIGHT_IDX * 4), 500);
	}
	else{
		Xil_Out32(XPAR_RGB_LED_CONTROLLER_1_S00_AXI_BASEADDR + (RED_IDX 	 * 4), 0);
		Xil_Out32(XPAR_RGB_LED_CONTROLLER_1_S00_AXI_BASEADDR + (GREEN_IDX  * 4), 0);
		Xil_Out32(XPAR_RGB_LED_CONTROLLER_1_S00_AXI_BASEADDR + (BLUE_IDX 	 * 4), 255);
		Xil_Out32(XPAR_RGB_LED_CONTROLLER_1_S00_AXI_BASEADDR + (BRIGHT_IDX * 4), 500);
	}

}

// Update displays with the number of allowed and blocked packets
void updateDisplays(){
	Xil_Out8(XPAR_DISPLAYCONTROLLER_0_S00_AXI_BASEADDR+8,
		((NUM_PACKETS-NUM_BLOCKED)%10 & 15) | (((NUM_PACKETS-NUM_BLOCKED)%100)/10 & 15)<<4);

	Xil_Out8(XPAR_DISPLAYCONTROLLER_0_S00_AXI_BASEADDR+9,
		(((NUM_PACKETS-NUM_BLOCKED)%1000)/100 & 15) | (((NUM_PACKETS-NUM_BLOCKED)%10000)/1000 & 15)<<4);

	Xil_Out8(XPAR_DISPLAYCONTROLLER_0_S00_AXI_BASEADDR+10,
		(NUM_BLOCKED%10 & 15) | ((NUM_BLOCKED%100)/10 & 15)<<4);

	Xil_Out8(XPAR_DISPLAYCONTROLLER_0_S00_AXI_BASEADDR+11,
		((NUM_BLOCKED%1000)/100 & 15) | ((NUM_BLOCKED%10000)/1000 & 15)<<4);
}

// Update the displays and leds
void updateUI(){
	update_rgb1();
	update_rgb2();
	updateDisplays();
}

// Save new packet on an array
void newPacket(int ip){
	last_ips[current_offset] = ip;
	current_offset = (current_offset+1)%MAX_NUMBER_OF_IPS;

	if(number_of_saved_ips<MAX_NUMBER_OF_IPS)
		number_of_saved_ips++;

	NUM_PACKETS++;
	updateUI();
}

// Process new blocked packet
void newBlockedPacket(int ip){
	NUM_BLOCKED++;
	LAST_PACKET_BLOCKED = 1;
	newPacket(ip);
}

// Process new allowed packet
void newAllowedPacket(int ip){
	LAST_PACKET_BLOCKED = 0;
	newPacket(ip);
}


// Returns 1 if IP should be blocked, 0 otherwise
int shouldBlockIp(int ip){

	if(number_of_saved_ips<MAX_NUMBER_OF_IPS)
		return 0;

	for(int i=0; i<MAX_NUMBER_OF_IPS; i++){
		if(last_ips[i]!=ip)
			return 0;
	}

	return 1;
}

// Reset the blocked IP counters
void resetCounters(){
	unsigned int value = XGpio_ReadReg(XPAR_AXI_GPIO_0_BASEADDR, XGPIO_DATA_OFFSET);

	// Center button
	if(value & 1<<4){
		NUM_BLOCKED = 0;
		NUM_PACKETS = 0;
		LAST_PACKET_BLOCKED = 0;
		number_of_saved_ips = 0;
		current_offset = 0;
		updateUI();
	}

	/*
	 * Clear interrupt status (in service) bit at push buttons GPIO
	 */
	XGpio_WriteReg(XPAR_AXI_GPIO_0_BASEADDR, XGPIO_ISR_OFFSET, XGPIO_IR_CH1_MASK);
}

// Clean the saved ip's on the last period
void cleanIps(){
	number_of_saved_ips = 0;
	current_offset = 0;
}

// Timer interrupt for clean the packets on the last period
void timerInterrupt(){

	current_second ++;
	if(current_second==SECONDS_TO_SAVE_IPS){
		current_second=0;
		cleanIps();
	}
}

