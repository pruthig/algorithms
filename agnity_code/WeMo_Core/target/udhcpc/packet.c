#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <features.h>
#if __GLIBC__ >=2 && __GLIBC_MINOR >= 1
#include <netpacket/packet.h>
#include <net/ethernet.h>
#else
#include <asm/types.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#endif
#include <errno.h>

#include "packet.h"
#include "debug.h"
#include "dhcpd.h"
#include "options.h"


void init_header(struct dhcpMessage *packet, char type)
{
	memset(packet, 0, sizeof(struct dhcpMessage));
	switch (type) {
	case DHCPDISCOVER:
	case DHCPREQUEST:
	case DHCPRELEASE:
	case DHCPINFORM:
		packet->op = BOOTREQUEST;
		break;
	case DHCPOFFER:
	case DHCPACK:
	case DHCPNAK:
		packet->op = BOOTREPLY;
	}
	packet->htype = ETH_10MB;
	packet->hlen = ETH_10MB_LEN;
	packet->cookie = htonl(DHCP_MAGIC);
	packet->options[0] = DHCP_END;
	add_simple_option(packet->options, DHCP_MESSAGE_TYPE, type);
#ifdef		CONFIG_YAHOO_BB
	if ( DHCPDISCOVER == type ) {
		add_simple_option ( packet->options , 0x74 , 0x01 );
	}
#endif		/* end of CONFIG_YAHOO_BB */
}


/* read a packet from socket fd, return -1 on read error, -2 on packet error */
int get_packet(struct dhcpMessage *packet, int fd)
{
	int bytes;
	int i;
	const char broken_vendors[][8] = {
		"MSFT 98",
		""
	};
	char unsigned *vendor;

	memset(packet, 0, sizeof(struct dhcpMessage));
	bytes = read(fd, packet, sizeof(struct dhcpMessage));
	if (bytes < 0) {
		DEBUG(LOG_INFO, "couldn't read on listening socket, ignoring");
		return -1;
	}

	if (ntohl(packet->cookie) != DHCP_MAGIC) {
		LOG(LOG_ERR, "received bogus message, ignoring");
		return -2;
	}
	DEBUG(LOG_INFO, "Received a packet");
	
	if (packet->op == BOOTREQUEST && (vendor = get_option(packet, DHCP_VENDOR))) {
		for (i = 0; broken_vendors[i][0]; i++) {
			if (vendor[OPT_LEN - 2] == (unsigned char) strlen(broken_vendors[i]) &&
			    !strncmp(vendor, broken_vendors[i], vendor[OPT_LEN - 2])) {
			    	DEBUG(LOG_INFO, "broken client (%s), forcing broadcast",
			    		broken_vendors[i]);
			    	packet->flags |= htons(BROADCAST_FLAG);
			}
		}
	}
			    	

	return bytes;
}


u_int16_t checksum(void *addr, int count)
{
	/* Compute Internet Checksum for "count" bytes
	 *         beginning at location "addr".
	 */
	register int32_t sum = 0;
	u_int16_t *source = (u_int16_t *) addr;

	while (count > 1)  {
		/*  This is the inner loop */
		sum += *source++;
		count -= 2;
	}

	/*  Add left-over byte, if any */
	if (count > 0) {
		/* Make sure that the left-over byte is added correctly both
		 * with little and big endian hosts */
		u_int16_t tmp = 0;
		*(unsigned char *) (&tmp) = * (unsigned char *) source;
		sum += tmp;
	}
	/*  Fold 32-bit sum to 16 bits */
	while (sum >> 16)
		sum = (sum & 0xffff) + (sum >> 16);

	return ~sum;
}


/* Constuct a ip/udp header for a packet, and specify the source and dest hardware address */
int raw_packet(struct dhcpMessage *payload, u_int32_t source_ip, int source_port,
		   u_int32_t dest_ip, int dest_port, unsigned char *dest_arp, int ifindex)
{
	int fd;
	int result;
	struct sockaddr_ll dest;
	struct udp_dhcp_packet packet;
#ifdef		CONFIG_YAHOO_BB
	unsigned char		*TmPtr = NULL;
	unsigned int		i = 0 , OptionLength = 999 , PayloadLength = 999;
	unsigned int		UselessPayloadLength = 0;
#endif		/* end of CONFIG_YAHOO_BB */

	if ((fd = socket(PF_PACKET, SOCK_DGRAM, htons(ETH_P_IP))) < 0) {
		DEBUG(LOG_ERR, "socket call failed: %s", strerror(errno));
		return -1;
	}
	
	memset(&dest, 0, sizeof(dest));
	memset(&packet, 0, sizeof(packet));
	
	dest.sll_family = AF_PACKET;
	dest.sll_protocol = htons(ETH_P_IP);
	dest.sll_ifindex = ifindex;
	dest.sll_halen = 6;
	memcpy(dest.sll_addr, dest_arp, 6);
	if (bind(fd, (struct sockaddr *)&dest, sizeof(struct sockaddr_ll)) < 0) {
		DEBUG(LOG_ERR, "bind call failed: %s", strerror(errno));
		close(fd);
		return -1;
	}

#ifdef		CONFIG_YAHOO_BB
	TmPtr = (unsigned char *) payload->options;
	if ( ( NULL != payload ) && ( NULL != TmPtr ) ) {
		for ( i = 0 ; i < 303 ; i++ ) {
			if ( DHCP_END == *( TmPtr + i ) ) {
				if ( ( DHCP_PADDING == *( TmPtr + i + 1 ) ) &&
					( DHCP_PADDING == *( TmPtr + i + 2 ) ) &&
					( DHCP_PADDING == *( TmPtr + i + 3 ) ) &&
					( DHCP_PADDING == *( TmPtr + i + 4 ) ) ) {
					OptionLength = ( i + 1 ) + 4;		// Total option length = options + DHCP_END + 4*DHCP_PADDING
					if ( OptionLength >= 303 ) {
						OptionLength = 999;
						PayloadLength = 999;
					} else {
						PayloadLength = sizeof ( struct dhcpMessage ) - 308 + OptionLength;		// DHCP message length
						UselessPayloadLength = 308 - OptionLength;
					}
					break;
				}
			}
		}
	}
#endif		/* end of CONFIG_YAHOO_BB */

	packet.ip.protocol = IPPROTO_UDP;
	packet.ip.saddr = source_ip;
	packet.ip.daddr = dest_ip;
	packet.udp.source = htons(source_port);
	packet.udp.dest = htons(dest_port);
#ifdef		CONFIG_YAHOO_BB
	if ( 999 != PayloadLength ) {
		packet.udp.len = htons ( sizeof ( packet.udp ) + PayloadLength );
	} else {
		packet.udp.len = htons ( sizeof ( packet.udp ) + sizeof ( struct dhcpMessage ) );
	}
#else
	packet.udp.len = htons(sizeof(packet.udp) + sizeof(struct dhcpMessage)); /* cheat on the psuedo-header */
#endif		/* end of CONFIG_YAHOO_BB */
	packet.ip.tot_len = packet.udp.len;
#ifdef		CONFIG_YAHOO_BB
	if ( 999 != PayloadLength ) {
		memcpy ( &(packet.data) , payload , PayloadLength );
		packet.udp.check = checksum ( &packet , sizeof ( struct udp_dhcp_packet ) - UselessPayloadLength );
	} else {
	memcpy(&(packet.data), payload, sizeof(struct dhcpMessage));
	packet.udp.check = checksum(&packet, sizeof(struct udp_dhcp_packet));
	}
#else
	memcpy(&(packet.data), payload, sizeof(struct dhcpMessage));
	packet.udp.check = checksum(&packet, sizeof(struct udp_dhcp_packet));
#endif		/* end of CONFIG_YAHOO_BB */
	
#ifdef		CONFIG_YAHOO_BB
	if ( 999 != PayloadLength ) {
		packet.ip.tot_len = htons ( sizeof ( struct udp_dhcp_packet )- UselessPayloadLength );
	} else {
	packet.ip.tot_len = htons(sizeof(struct udp_dhcp_packet));
	}
#else
	packet.ip.tot_len = htons(sizeof(struct udp_dhcp_packet));
#endif		/* end of CONFIG_YAHOO_BB */
	packet.ip.ihl = sizeof(packet.ip) >> 2;
	packet.ip.version = IPVERSION;
	packet.ip.ttl = IPDEFTTL;
	packet.ip.check = checksum(&(packet.ip), sizeof(packet.ip));

#ifdef		CONFIG_YAHOO_BB
	if ( 999 != PayloadLength ) {
		result = sendto(fd, &packet, sizeof(struct udp_dhcp_packet)- UselessPayloadLength, 0, (struct sockaddr *) &dest, sizeof(dest));
	} else {
	result = sendto(fd, &packet, sizeof(struct udp_dhcp_packet), 0, (struct sockaddr *) &dest, sizeof(dest));
	}
#else
	result = sendto(fd, &packet, sizeof(struct udp_dhcp_packet), 0, (struct sockaddr *) &dest, sizeof(dest));
#endif		/* end of CONFIG_YAHOO_BB */
	if (result <= 0) {
		DEBUG(LOG_ERR, "write on socket failed: %s", strerror(errno));
	}
	close(fd);
	return result;
}


/* Let the kernel do all the work for packet generation */
int kernel_packet(struct dhcpMessage *payload, u_int32_t source_ip, int source_port,
		   u_int32_t dest_ip, int dest_port)
{
	int n = 1;
	int fd, result;
	struct sockaddr_in client;
	
	if ((fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		return -1;
	
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *) &n, sizeof(n)) == -1)
		return -1;

	memset(&client, 0, sizeof(client));
	client.sin_family = AF_INET;
	client.sin_port = htons(source_port);
	client.sin_addr.s_addr = source_ip;

	if (bind(fd, (struct sockaddr *)&client, sizeof(struct sockaddr)) == -1)
		return -1;

	memset(&client, 0, sizeof(client));
	client.sin_family = AF_INET;
	client.sin_port = htons(dest_port);
	client.sin_addr.s_addr = dest_ip; 

	if (connect(fd, (struct sockaddr *)&client, sizeof(struct sockaddr)) == -1)
		return -1;

	result = write(fd, payload, sizeof(struct dhcpMessage));
	close(fd);
	return result;
}	
