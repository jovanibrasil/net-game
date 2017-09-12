/*
 * Trabalho de Redes de Computadores II - CC/PUCRS
 *
 * Alunos: João Chagas e Jovani Brasil
 * Professora: Ana Benso
 *
 *
 */

#ifndef _NETUTILS_H_
#define _NETUTILS_H_

#include <netinet/in.h>

/**********************************************************************************************************
 *
 * Funções para busca do mac no enlace do host.
 *
 **********************************************************************************************************/

int send_arp_request(unsigned char *dst_ip, unsigned char *mac_address, char interface[]); 
int get_mac_address(char *ip_dst, char *mac, char interface[]);


/**********************************************************************************************************
 *
 * Funções para envio de dados.
 *
 **********************************************************************************************************/

int send_udp(int sock, char *dst_ip, unsigned char *data, int data_size, int default_port, char interface[]);


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
