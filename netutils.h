/*
 * Autor: Jovani Brasil.
 *
 */

#ifndef _NETUTILS_H_
#define _NETUTILS_H_

#include <netinet/in.h>

/**********************************************************************************************************
 *
 * Funções para envio de dados.
 *
 **********************************************************************************************************/

int send_udp(int sock, char *dst_ip, u_char mac_dst[6], unsigned char *data, int data_size, int default_port, char interface[]);


/*********************************************************************************************************
 *
 * Funções para recebimento de dados.
 *
 * *******************************************************************************************************/

int get_udp(unsigned char *buff, int offset, int type, int default_port, unsigned char *data);

int get_ipv4(unsigned char *buff, int offset, uint32_t host_ip, int type, int default_port, unsigned char *data, char *dst_ip);
int get_ethernet(unsigned char *buff, char *host_mac, uint32_t host_ip, int type, int default_port, unsigned char *data, char *dst_ip, unsigned char *dst_mac);	
int get_packet(int sock, int default_port, char interface[], int type, unsigned char *data, char *dst_ip, unsigned char *dst_mac);


#endif
