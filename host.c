/*
 * Autor: Jovani Brasil.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>

/* Diretorios: net, netinet, linux contem os includes que descrevem as estruturas de dados do header dos protocolos.
 *
 * net/if.h  		- 	Estruturas para gerencia de interfaces de rede.
 * netinet/in_systm.h 	- 	Tipos de dados. 
 * netinet/ether.h 	- 	Estrutura do header ethernet.
 * netinet/in.h 	-	Definicao de protocolos. 
 * arpa/inet.h 		- 	Funcoes para manipulacao de enderecos IP.
 */

#include <net/if.h>  
#include <netinet/in_systm.h> 
#include <netinet/ether.h> 
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <netinet/ip_icmp.h>
#include <netinet/icmp6.h>
#include <netinet/in.h> 
#include <arpa/inet.h> 


#include <linux/if_packet.h>
#include <net/ethernet.h>

#include "nettypes.h"
#include "netutils.h"
#include "game.h"

/*
 * Envia um pacote udp sem dados extras. Os únicos dados
 * são os contidos no cabeçalho de dados.
 *
 */
void send_udp_without_extra_data(int sock, int cmd, int user_id){
		
		// Alocando e inicializando buffer de dados.		
		int header_size = sizeof(struct data_t);

		unsigned char *data = (unsigned char*)malloc(header_size); 
		memset(data, 0, header_size);

		struct data_t *data_header = (struct data_t*)(data);
		data_header->cmd = cmd;
		data_header->id = user_id;
		data_header->ack = 0;
		data_header->off = 0;

		send_udp(sock, SERVER_IP, data, header_size, PORT, ITF);			

}


/*
 * Envia um pacote udp sem dados extras. Além dos dados
 * contidos no cabeçalho de dados, existe mais um bloco
 * em anexo.
 *
 */
void send_udp_with_extra_data(int sock, int cmd, int user_id, char param1[]){

		// Alocando e inicializando buffer de dados.		
		int header_size = sizeof(struct data_t);
		int payload_size = strlen(param1) * sizeof(unsigned char);

		unsigned char *data = (unsigned char*)malloc(header_size + payload_size); 
		memset(data, 0, header_size + payload_size);

		struct data_t *data_header = (struct data_t*)(data);
		data_header->cmd = cmd;
		data_header->id = user_id;
		data_header->ack = 0;
		data_header->off = 0;

		memcpy(data+header_size, param1, payload_size);	

		send_udp(sock, SERVER_IP, data, header_size + payload_size, PORT, ITF);		
				
}

/*
 * Recebe um pacote udp do tipo especificado.
 *
 */
void get_udp_packet(int sock, int cmd, char *dst_ip, char *dst_mac){

	int offset = 0, data_size;				

	unsigned char *data = (unsigned char*)malloc(300); 
	memset(data, 0, 300);			

	if((data_size = get_packet(sock, PORT, ITF, cmd, data, dst_ip, dst_mac)) > 0){
				
		struct data_t data_h;
		memcpy(&data_h, data, sizeof(struct data_t));
		
		offset += sizeof(struct data_t);
		int ds = data_size - sizeof(struct data_t);

		char *input = malloc(sizeof(char) * ds);
		printf("Tamanho da msg: %d\n", ds);
		memcpy(input, &data[offset], ds);							
		printf("%s", input);

	}

}

/*
 * Chamada principal.
 *
 */

int main(int argc,char *argv[]){

	int ssock, rsock;
	unsigned char *data;

	// Abrindo socket para recebimento de dados.
	if((rsock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0) {	
		printf("Erro na criacao do socket.\n");
		exit(1);
	}

	// Abrindo socket para envio de dados.
	if((rsock = socket(AF_PACKET, SOCK_RAW, IPPROTO_IP)) < 0)
		printf("[x] Erro na abertura do socket.\n");

	printf("Trabalho de laboratório de redes 2017/2\n");

	/*

	int online = 0;
	int user_id;

	while(1){

		// Se não está logado, faz processo de login.	
		if(!online){

			while(1){

				// Solicita nome ao usuário.
		
				char input_login[50];				
			
				printf("Informe um Login(máximo 50 caracteres): \n");
	  			scanf("%s", input_login);

				// Envia pacote solicitando login.
			
				// Alocando e inicializando buffer de dados.		
				int header_size = sizeof(struct data_t);
				int payload_size = strlen(input_login) * sizeof(unsigned char);

				data = (unsigned char*)malloc(header_size + payload_size); 
				memset(data, 0, header_size + payload_size);

				struct data_t *data_header = (struct data_t*)(data);
				data_header->cmd = 0;
				data_header->id = -1;
				data_header->ack = 0;
				data_header->off = 0;
	
				memcpy(data+header_size, input_login, payload_size);	

				send_udp(sock, SERVER_IP ,data, header_size + payload_size, PORT, ITF);
				
				// Limpa buffer de dados.
				// TODO Usar tamanho máximo fixo do buffer? Eis a questão. Justificar o tamanho.
				int offset = 0, data_size;				
				memset(data, 0, 64);

				// Captura um resposta de login.	
				if((data_size = get_packet(sockd, PORT, ITF, 0, data, dst_ip, dst_mac)) > 0){
					printf("Pacote recebido!\n");
					struct data_t data_h;

					memcpy(&data_h, data, sizeof(struct data_t));
					offset += sizeof(struct data_t);
					int ds = data_size - sizeof(struct data_t);

					if(data_h.cmd == LOGIN){

						// Logou com sucesso
						if(data_h.ack == 1){
							printf("Logou com sucesso!!\n");
							online = 1;
							user_id = data_h.id;
							break;
						}else{
							printf("Falhar ao logar!!\n");
						}
					}
				}
			}
		}
		else{

	*/

	char *dst_ip = malloc(16);
	unsigned char *dst_mac = malloc(6);

	char *input = malloc(sizeof(char) *50);
	char *cmd = malloc(sizeof(char)*50); 
	// Server receiver simulation - Server layer
	while(1){

		char *param1 = malloc(sizeof(char) *50); 
		char *param2 = malloc(sizeof(char) *50);   

		printf("- Comando:~$\n"); 
		fgets(input, 50, stdin);
		sscanf(input, "%s %s %s", cmd, param1, param2);  

		int cmd_type = get_cmd_type(cmd);	

		switch(cmd_type){
			case EXAMINE:
				send_udp_with_extra_data(ssock, cmd_type, 0, param1);
				break;
			case INVENTORY:
				send_udp_without_extra_data(ssock, cmd_type, 0);
				break;
			case CATCH:
				send_udp_with_extra_data(ssock, cmd_type, 0, param1);
				break;
			case DROP:
				send_udp_with_extra_data(ssock, cmd_type, 0, param1);
				break;
			case MOVE:
				send_udp_with_extra_data(ssock, cmd_type, 0, param1);
				break;
			case HELP:
				send_udp_without_extra_data(ssock, cmd_type, 0);
				break;
			default:
				printf("Comando desconhecido.\n");
		}

		get_udp_packet(rsock, cmd_type, dst_ip, dst_mac);

	}

	close(ssock);
	close(rsock);

	return 0;

}
