/*
 * Trabalho de Redes de Computadores II - CC/PUCRS
 *
 * Alunos: João Chagas e Jovani Brasil
 * Professora: Ana Benso
 *
 *
 */


#ifndef _NETTYPES_
#define _NETTYPES_

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>  
#include <netinet/in_systm.h> 
#include <netinet/ether.h> 
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/in.h> 
#include <arpa/inet.h> 

#include <linux/if_packet.h>
#include <net/ethernet.h>

#define BUFFSIZE 1518
#define SHOW_HEADERS2

#define PORT 23453
// #define ITF "enp7s0"
#define ITF "lo"
#define SERVER_IP "127.0.0.1"

#define ETH_LENGTH 6

struct data_t {

	int cmd; // Comando a ser executado.
	int id;  // Identificação da sessão aberta.
	int ack; // Flack de confirmação

	int off; // Para carregar duas porções de dados no mesmo pack.

};

struct arp_header
{
        unsigned short ar_hrd;
        unsigned short ar_pro;
        unsigned char ar_hln;
        unsigned char  ar_pln;
        unsigned short ar_op;
        unsigned char ar_sha[6];
        unsigned char ar_sip[4];
        unsigned char ar_tha[6];
        unsigned char ar_tip[4];
};



#endif
