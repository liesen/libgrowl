/*
 * 
 * Library implementing the Growl protocol
 * 
 * Author Antti Holvikari <anttih@gmail.com>
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "libgrowl.h"

int growl_register_app(GrowlPacketRegister *reg)
{
	int bytes_sent;
	GrowlPacket *gp;
	
	gp = (GrowlPacket *) malloc(sizeof(GrowlPacket));
	
	growl_create_packet(reg, gp);
	bytes_sent = growl_send_packet(gp);
	
	free(gp->data);
	free(gp);
	
	printf("Sent %d bytes\n", bytes_sent);
	return 1;
}

int growl_create_packet(GrowlPacketRegister *packet, GrowlPacket *gp)
{
	int i;
	unsigned short len;
	unsigned short len_htons;
	unsigned short app_name_len;
	unsigned char *data;
	
	/* this we know for sure */
	gp->len = 6;
	gp->len += packet->app_name_len;
	
	for (i = 0; i < packet->notifications_num; i++) {
		gp->len += strlen(packet->notifications[i]);
		gp->len += sizeof(unsigned short);
	}
	
	/* find packet size */
	gp->data = (unsigned char *) malloc(gp->len);
	
	data = gp->data;
	
	/* Version */
	data = (unsigned char *) memcpy(data, &packet->ver, sizeof(packet->ver));
	data += sizeof(packet->ver);

	/* Type */
	data = (unsigned char *) memcpy(data, &packet->type, sizeof(packet->type));
	data += sizeof(packet->type);
	
	/* network byte-order */
	app_name_len = htons(packet->app_name_len);
	
	data = (unsigned char *) memcpy(data, &app_name_len, sizeof(unsigned short));
	data += sizeof(unsigned short);
	
	/* nall (number of notifications in the list) */
	data = (unsigned char *) memcpy(data, (unsigned char *) &packet->notifications_num, sizeof(char));
	data += sizeof(char);
	
	/* ndef (number of notifications enabled by default) */
	data = (unsigned char *) memcpy(data, (unsigned char *) &packet->notifications_num, sizeof(char));
	data += sizeof(char);
	
	memcpy(data, packet->app_name, packet->app_name_len);
	data += packet->app_name_len;
	
	for (i = 0; i < packet->notifications_num; i++) {
		len = strlen(packet->notifications[i]);
		len_htons = htons(len);
		
		data = (unsigned char *) memcpy(data, &len_htons, sizeof(unsigned short));
		data += sizeof(unsigned short);
		
		data = (unsigned char *) memcpy(data, packet->notifications[i], len);
		data += len;
	}
	
	return 1;
}

int growl_send_packet(GrowlPacket *packet)
{
	int bytes_sent;
	int sockfd;
	socklen_t to_len;
	struct sockaddr_in addr;
	
	/*
	int fromlen;
	void *recv_buf;
	int recv_buf_len = 512;
	*/
	sockfd = socket(PF_INET, SOCK_DGRAM, 0);

	addr.sin_family = AF_INET;                  /* host byte order */
	addr.sin_port = htons(GROWL_DEFAULT_PORT);  /* short, network byte order */
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	memset(addr.sin_zero, 0, sizeof(addr.sin_zero));
	
	to_len = sizeof(struct sockaddr);
	
	/* actually send the packet */
	bytes_sent = sendto(sockfd, packet->data, packet->len, 0, (struct sockaddr *) &addr, to_len);
	
	/*
	if (recvfrom(sockfd, recv_buf, recv_buf_len, 0, (struct sockaddr *) &addr, (unsigned *) &fromlen) < 0) {
		printf("Error reading from remote host\n");
	}
	*/
	return bytes_sent;
}