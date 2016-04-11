/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"

#include <string.h>

#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"

#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])

#define MAX_PAYLOAD_LEN 120

static struct uip_udp_conn *server_conn;
static uip_ipaddr_t base_addr;

PROCESS(udp_server_process, "UDP server process");
AUTOSTART_PROCESSES(&resolv_process,&udp_server_process);
/*---------------------------------------------------------------------------*/
static void
tcpip_handler(void)
{
  //static int seq_id;
  char buf[MAX_PAYLOAD_LEN], recv_buf[MAX_PAYLOAD_LEN];
  signed short rssi = 0;
  uip_ipaddr_t recv_addr;

  if(uip_newdata()) {
    ((char *)uip_appdata)[uip_datalen()] = 0;
    strcpy(recv_buf, (char *)uip_appdata);
    rssi = (signed short) packetbuf_attr(PACKETBUF_ATTR_RSSI);
    PRINTF("Server received: '%s' (RSSI: %d) from ", (char *)uip_appdata, rssi);
    PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
    uip_ipaddr_copy(&recv_addr, &UIP_IP_BUF->srcipaddr);
    PRINTF("\n\r");
    
    /************************Respond with system time (for sync)**************/
    uip_ipaddr_copy(&server_conn->ripaddr, &UIP_IP_BUF->srcipaddr);
    PRINTF("NOT Responding with message: ");
    sprintf(buf, "%lu", clock_seconds());
    PRINTF("%s\n\r", buf);
    //uip_udp_packet_send(server_conn, buf, strlen(buf));
    /*************************************************************************/


    /******************Forward data to base node******************************/
    uip_ipaddr_copy(&server_conn->ripaddr, &base_addr);
    sprintf(buf, "%02x%02x%02x%02x: %s, %d", 0xff&((&recv_addr)->u16[6]),
            (&recv_addr)->u16[6] >> 8,
            0xff&(&recv_addr)->u16[7], 
            (&recv_addr)->u16[7] >> 8, 
            recv_buf, rssi);
    PRINTF("Sending to base: %s\n\r", buf);
    uip_udp_packet_send(server_conn, buf, strlen(buf));
    /*************************************************************************/
    /* Restore server connection to allow data from any node */
    memset(&server_conn->ripaddr, 0, sizeof(server_conn->ripaddr));
  }
}
/*---------------------------------------------------------------------------*/
static void
print_local_addresses(void)
{
  int i;
  uint8_t state;

  PRINTF("Server IPv6 addresses: ");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused &&
       (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
      PRINT6ADDR(&uip_ds6_if.addr_list[i].ipaddr);
      PRINTF("\n\r");
    }
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_server_process, ev, data)
{
#if UIP_CONF_ROUTER
  uip_ipaddr_t ipaddr;
#endif /* UIP_CONF_ROUTER */

  PROCESS_BEGIN();
  PRINTF("UDP server started\n\r");

#if RESOLV_CONF_SUPPORTS_MDNS
  resolv_set_hostname("contiki-udp-server");
#endif

#if UIP_CONF_ROUTER
  uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
  uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);
#endif /* UIP_CONF_ROUTER */

  print_local_addresses();
  NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, 5);
  NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, 17);

  //Broadcast on 4000
  //Listen on 3000
  server_conn = udp_new(NULL, UIP_HTONS(4000), NULL);
  udp_bind(server_conn, UIP_HTONS(3000));

/********define the base address IP address here******************************/
  uip_ip6addr(&base_addr, 0xfe80, 0, 0, 0, 0x212, 0x4b00, 0x799, 0xa283);
/*****************************************************************************/

  while(1) {
    PROCESS_YIELD();

	//Wait for tcipip event to occur
    if(ev == tcpip_event) {
      tcpip_handler();
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
