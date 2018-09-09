
#include <stdio.h>
#include <string.h>
#include "BloomFilterUtils.h"
#include "processPackets.h"
#include "MemoryUtils.h"

#include "lwip/err.h"
#include "lwip/tcp.h"
#if defined (__arm__) || defined (__aarch64__)
#include "xil_printf.h"
#endif

#include "xil_io.h"


err_t recv_callback(void *arg, struct tcp_pcb *tpcb,
                               struct pbuf *p, err_t err)
{
	/* do not read the packet if we are not in ESTABLISHED state */
	if (!p) {
		tcp_close(tpcb);
		tcp_recv(tpcb, NULL);
		return ERR_OK;
	}

	/* indicate that the packet has been received */
	tcp_recved(tpcb, p->len);

	/* in this case, we assume that the payload is < TCP_SND_BUF */
	if (tcp_sndbuf(tpcb) > p->len) {
		xil_printf("%d.%d.%d.%d -> ", ip4_addr1(&tpcb->remote_ip.addr), ip4_addr2(&tpcb->remote_ip.addr),
					ip4_addr3(&tpcb->remote_ip.addr), ip4_addr4(&tpcb->remote_ip.addr));

		// Check if the IP is blocked
		int isBlocked = isInFilter(tpcb->remote_ip.addr);

		if(((isBlocked & 1) == 1) && (isInMemory(tpcb->remote_ip.addr) & 1)==1){
			xil_printf("Blocked IP\n");
			newBlockedPacket(tpcb->remote_ip.addr);
		}
		else{
			// Check if the IP should be blocked
			int blockIp = shouldBlockIp(tpcb->remote_ip.addr);

			if(blockIp==1){
				xil_printf("Blocking IP!\n");
				int write = writeInFilter(tpcb->remote_ip.addr);
				write = writeInMemory(tpcb->remote_ip.addr);
				if(write != 0)
					xil_printf("Something went wrong and the IP was not written in memory!\n");
				newBlockedPacket(tpcb->remote_ip.addr);
			}
			else{
				xil_printf("Allowed!\n");
				newAllowedPacket(tpcb->remote_ip.addr);
			}
		}

	} else
		xil_printf("No space in tcp_sndbuf\n");

	/* free the received pbuf */
	pbuf_free(p);

	return ERR_OK;
}

err_t accept_callback(void *arg, struct tcp_pcb *newpcb, err_t err)
{
	static int connection = 1;

	/* set the receive callback for this connection */
	tcp_recv(newpcb, recv_callback);

	/* just use an integer number indicating the connection id as the
	   callback argument */
	tcp_arg(newpcb, (void*)(UINTPTR)connection);

	/* increment for subsequent accepted connections */
	connection++;

	return ERR_OK;
}


int start_application(int port)
{
	struct tcp_pcb *pcb;
	err_t err;

	/* create new TCP PCB structure */
	pcb = tcp_new();
	if (!pcb) {
		xil_printf("Error creating PCB. Out of Memory\n\r");
		return -1;
	}

	/* bind to specified @port */
	err = tcp_bind(pcb, IP_ADDR_ANY, port);
	if (err != ERR_OK) {
		xil_printf("Unable to bind to port %d: err = %d\n\r", port, err);
		return -2;
	}

	/* we do not need any arguments to callback functions */
	tcp_arg(pcb, NULL);

	/* listen for connections */
	pcb = tcp_listen(pcb);
	if (!pcb) {
		xil_printf("Out of memory while tcp_listen\n\r");
		return -3;
	}

	/* specify callback to use for incoming connections */
	tcp_accept(pcb, accept_callback);

	xil_printf("TCP server started @ port %d\n\r", port);

	return 0;
}
