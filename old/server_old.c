/*
 * Trabalho de Redes de Computadores II - CC/PUCRS
 *
 * Alunos: João Chagas e Jovani Brasil
 * Professora: Ana Benso
 *
 *
 * server.c
 *
 * TODO Documentation.
 *
 *
 *
 *
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

#include "netutils.h"
#include "types.h"
#include "game.c"

void println(char *buff){

	int i;
	for(i=0;; i++){
		if(buff[i] == '\0'){
			printf("\n");
			return;	
		}
		printf("%c", buff[i]);
	}
}

void print_mac(unsigned char *mac){
	printf("  %02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

int login(int sock, unsigned char *data, int data_size, char *user_ip, unsigned char *user_mac, unsigned char *users_info){

	printf("Data size %d\n", data_size);
	// Separa cabeçalho
	struct data_t data_h;
	memcpy(&data_h, data, sizeof(struct data_t));	
	
		
	int offset = sizeof(struct data_t);
	int name_size = data_size - sizeof(struct data_t);

	printf("Name size %d\n", name_size);

	char *input_login = malloc(sizeof(char) * name_size);

	memcpy(input_login, &data[offset], name_size);	
	
	//put_login[name_size] = '\0';

	printf("[?] Usuario pedindo login:");
	println(input_login);

	// Verifica se já existe o login informado.
	int verify = find_user(input_login);
	int saved;

	int ack = 0;
	int user_id = -1;

	// Se for, salva.	
	if(verify < 0){
		printf("[v] Usuario aceito.\n");
		user_id = save_user(input_login);

		printf("[v] ID: %d\n", user_id);
	
		memcpy(&users_info[user_id*(16+6)], user_ip, 16);	
		memcpy(&users_info[user_id*(16+6)+16], user_mac, 6);	

		ack = 1;
	}else{
		// TODO PQ?
		printf("[x] Usuario não aceito!\n");
		ack = 0;
	}

	// Manda retorno, com ack positivo ou negativo	
	int header_size = sizeof(struct data_t);

	data = (unsigned char*)malloc(header_size); 
	memset(data, 0, header_size);

	struct data_t *data_header = (struct data_t*)(data);
	data_header->cmd = 0;
	data_header->id = user_id; 
	data_header->ack = ack;
	data_header->off = 0;

	printf("[v] Enviando confirmação para o host.\n");
        printf("%d\n", PORT);
	send_udp(sock, user_ip, data, header_size, PORT, ITF);		

	return user_id;

}

void send_examine(int sock, unsigned char *data, int data_size, char *dst_ip,  unsigned char *users){

	struct data_t data_h;
	memcpy(&data_h, data, sizeof(struct data_t));	
	int offset = sizeof(struct data_t);
	int name_size = data_size - sizeof(struct data_t);

	char *value = malloc(sizeof(char) * name_size);
	//value[name_size] = '\0';

	memcpy(value, &data[offset], name_size);	
			
	// TODO Troca nomes

	printf("[v] Respondendo requisição de examinar.\n");

	struct Jogador *player = (struct Jogador*)users+data_h.id*sizeof(struct Jogador);
	
	unsigned char *help_msg = malloc(200);
	unsigned char *help_data = malloc(300);

	printf("%s\n", value);

	int payload_size = examinar(value, *player, help_msg);	
	

	printf("%s\n", help_msg);
	
	int header_size = sizeof(struct data_t);

	memset(help_data, 0, 0);

	struct data_t *data_header = (struct data_t*)(help_data);
	data_header->cmd = data_h.cmd;
	data_header->id = data_h.id;
	data_header->ack = 1;
	data_header->off = 0;

	memcpy(&help_data[header_size], help_msg, payload_size);	

	send_udp(sock, dst_ip, help_data, header_size + payload_size, PORT, ITF);		

}
	
void send_move(int sock, unsigned char *data, int data_size, char *dst_ip, unsigned char *users){

	struct data_t data_h;
	memcpy(&data_h, data, sizeof(struct data_t));	
	int offset = sizeof(struct data_t);
	int name_size = 1 + data_size - sizeof(struct data_t);

	char *value = malloc(sizeof(char) * name_size);
	value[name_size] = '\0';

	memcpy(value, &data[offset], name_size);	
			
	// TODO Troca nomes

	printf("[v] Respondendo requisição de mover.\n");

	struct Jogador *player = (struct Jogador*)users+data_h.id*sizeof(struct Jogador);
	
	unsigned char *help_msg = malloc(200);
	unsigned char *help_data = malloc(300);

	int payload_size = move(*player, value, help_msg); 	
	
	int header_size = sizeof(struct data_t);

	memset(help_data, 0, 0);

	struct data_t *data_header = (struct data_t*)(help_data);
	data_header->cmd = data_h.cmd;
	data_header->id = data_h.id;
	data_header->ack = 1;
	data_header->off = 0;

	memcpy(&help_data[header_size], help_msg, payload_size);	

	// TODO Sucesso: envia msg para todos na sala.

	send_udp(sock, dst_ip, help_data, header_size + payload_size, PORT, ITF);		



}

void send_catch(){


}

void send_drop(){



}

void send_inventory(int sock, int user_id, int cmd, char *dst_ip, unsigned char *users){

	// TODO Troca nomes

	printf("[v] Respondendo requisição de invetorio.\n");

	struct Jogador *player = (struct Jogador*)users+user_id*sizeof(struct Jogador);
	
	unsigned char *help_msg = malloc(200);
	unsigned char *help_data = malloc(300);

	int payload_size = inventario(*player, help_msg);	
	int header_size = sizeof(struct data_t);

	memset(help_data, 0, 0);

	struct data_t *data_header = (struct data_t*)(help_data);
	data_header->cmd = cmd;
	data_header->id = user_id;
	data_header->ack = 1;
	data_header->off = 0;

	memcpy(&help_data[header_size], help_msg, payload_size);	

	send_udp(sock, dst_ip, help_data, header_size + payload_size, PORT, ITF);		

}

void send_use(){


}

void send_speak(){


}


/*
 * O jogador envia uma mensagem de texto apenas para o jogador especificado, se ambos
 * estiverem na mesma sala.
 *
 */
void send_whisper(int sock, unsigned char *data, int data_size, unsigned char *users){




	/*
	char *value = malloc(sizeof(char) * name_size);
	value[name_size] = '\0';

	memcpy(value, &data[offset], name_size);	
			
	printf("[v] Respondendo requisição de cochichar.\n");

	struct Jogador *player = (struct Jogador*)users+user_id*sizeof(struct Jogador);
	
	unsigned char *help_msg = malloc(200);
	unsigned char *help_data = malloc(300);

	
	int header_size = sizeof(struct data_t);

	memset(help_data, 0, 0);

	struct data_t *data_header = (struct data_t*)(help_data);
	data_header->cmd = cmd;
	data_header->id = user_id;
	data_header->ack = 1;
	data_header->off = 0;

	memcpy(&help_data[header_size], value, payload_size);	

	dst_id = save_user(dst_name);
	player = (struct Jogador*)users+user_id*sizeof(struct Jogador);



	memcpy(dst_ip, &users_info[id*(16+6)], 16);	
	memcpy(dst_mac, &users_info[id*(16+6)+16], 6);		


	send_udp(sock, dst_ip, help_data, header_size + payload_size, PORT, ITF);		

*/
}

void send_help(int sock, int cmd, int user_id, char dst_ip[]){
	
	printf("[v] Respondendo requisição de ajuda.\n");

	unsigned char *help_msg = malloc(200);
	unsigned char *help_data = malloc(300);

	int payload_size = help(help_msg);	
	int header_size = sizeof(struct data_t);

	memset(help_data, 0, 0);

	struct data_t *data_header = (struct data_t*)(help_data);
	data_header->cmd = cmd;
	data_header->id = user_id;
	data_header->ack = 1;
	data_header->off = 0;

	memcpy(&help_data[header_size], help_msg, payload_size);	

	send_udp(sock, dst_ip, help_data, header_size + payload_size, PORT, ITF);		
	
}

int get_user_identity(unsigned char *users_info, int id, unsigned char *user_mac, char *ip){

	// Pega IP registrado na base para o id.
	char *user_ip_aux = malloc(16);

	
	//printf("");

	//for(i=0; i<)

	// Pega MAC registrado na base para o MAC.
	char *user_mac_aux = malloc(16);
	
	
// Compara MAC


}

