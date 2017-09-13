/*
 *
 * 
 *
 *
 * Considera-se que podem existir n objetos iguais em uma sala,
 * não diferindo em relação aos seus atributos. 
 *
 * It1 - respondendo
 *
 * TODO printf("Examinar [sala/objeto]\n");
 * TODO printf("Mover [N/S/L/O]\n");
 * TODO printf("Pegar [objeto]\n");
 * TODO printf("Largar [objeto]\n");
 * TODO printf("Inventário\n");
 * TODO printf("Usar [objeto] {alvo}\n");
 * TODO printf("Falar [texto]\n");
 * TODO printf("Cochichar [texto] [jogador]\n");
 * printf("Ajuda\n");	
 *
 *
 */

#include "game.h"

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
#include "nettypes.h"

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



/*
 * Examina item especifico ou a sala.
 *
 * Se o parametro object_nme for inválido, retorna -1.
 *
 * object_name deve ser "sala" ou algum objeto. 
 *
 *
 */


int server_show_inventary(int player_id, struct player *player_list, char *buffer, int buffer_offset){
   
    printf("Requisição de inventario do player %d.\n", player_id); 
    
    struct player *p = get_player_by_id(player_list, player_id);
        
    if(p != NULL){
        return game_show_inventary(p, buffer); 
    }

    return -1;

}

int server_examine(int player_id, char *object_name, struct player *player_list, char *buffer, int buffer_size){

 	char str[200];

	// Busca pelo id do objeto a ser buscado.
	int obj_type = get_obj_type(object_name);

	if(obj_type == INVALID_ARG){
		printf("[x] Requisisição com parâmetro errado.\n");
		return -1;
	}else{

		// Busca player pelo id.
		struct player *p = get_player_by_id(player_list, player_id);
     
    	if(p != NULL){

			if(obj_type == ROOM){
				
				printf("[v] Requisição para examinar sala %d.\n", obj_type);

				// Busca por item lanterna no inventario do jogador
				struct item *it = get_player_item_by_id(p, FLASHLIGHT);

				// Se não há luz e lanterna está desligada
				if(p->actual_room->light_state == LIGHT_OFF){
						if(it == NULL){
    						strcpy(str, "Não há condição de examinar a sala. A luz está apagada e você não tem uma lanterna.\n");
							int size = strlen(str);
    						memcpy(buffer, str, size);    
							return size;
						}
						else if(it->state == LIGHT_OFF){
							strcpy(str ,"Não há condição de examinar a sala. A luz está apagada e você tem uma lanterna, use-a.\n");
							int size = strlen(str);
    						memcpy(buffer, str, size);    
							return size;

						}
				}

				int len = strlen(p->actual_room->description);  

				memcpy(buffer, p->actual_room->description, len);	
		
				int offset = to_string_room_itens(p->actual_room, buffer+len, 0) + len;
								
				return to_string_room_doors(p->actual_room, buffer, offset);

			}else{
				
				printf("[v] Requisição para examinar objeto %d.\n", obj_type);

				// Busca objeto na sala em que o player está.
				struct item *it = get_room_item_by_type(p->actual_room, obj_type);  
			
				// Objeto também pode estar no inventário.
				if(!it)
					it = get_player_item_by_id(p, obj_type);

				if(it){
					strcpy(str, description(it->type));
					int size = strlen(str);
    				memcpy(buffer, str, size);    
					return size;
			 	}

				printf("[x] Um erro aconteceu ao examinar objetos do jogo.\n");
				return -1;
			}

		}else{
			printf("[x] Jogador não encontrado.");
			return INVALID_ARG; 
		}
	}
		

}

int server_catch(int player_id, char *object_name, struct player *player_list, char *buffer){

    printf("Requisição do player %d para coleta de item.\n", player_id);

 	char str[200];

	int object_id = get_obj_type(object_name);
    struct player *p = get_player_by_id(player_list, player_id);
     
    if(p != NULL){
        struct item *it = get_room_item_by_type(p->actual_room, object_id);  
        if(it){
            game_catch(p, it);
			printf("[v] Objeto inserido no inventario do player %d.\n", player_id);
			strcpy(str ,"Objeto inserido no inventario.\n");
        }else{
			printf("[x] Objeto não pode ser pego.\n");
			strcpy(str ,"Objeto não pode ser pego.\n");
		}
    }else{
		printf("[x] Usuário inválido.\n");
		strcpy(str ,"Objeto não pode ser pego.\n");
	}	

	int size = strlen(str);
	memcpy(buffer, str, size);    
	return size;
}

int server_drop(int player_id, char *object_name, struct player *players_list, char *buffer){
 
    printf("[v] Requisição do player %d para largar item.\n", player_id);

 	char str[200];

	int object_id = get_obj_type(object_name);
    struct player *p = get_player_by_id(players_list, player_id);
    
    if(p != NULL){
        struct item *it = get_player_item_by_id(p, object_id); 
        if(it){
            game_drop(p, it);
            strcpy(str, "Item removido do inventário.\n");
        }else{
			printf("[x] Item selecionado não encontrado pelo servidor!\n");
			strcpy(str, "Item não encontrado no servidor.\n");
		}
    }else{
		printf("[x] Player não encontrado pelo servidor!\n");
		strcpy(str, "Player não encontrado no servidor.\n");
	}

    int size = strlen(str);
	memcpy(buffer, str, size);    
	return size;
}

int server_move(int player_id, char dir_str[20], struct player *players_list, char *buffer){
 
    printf("[v] Requisição do player %d se mover.\n", player_id);

 	char str[200];

	printf("%s\n", dir_str);

	int dir_type = get_dir_type(dir_str);
	struct player *p = get_player_by_id(players_list, player_id);

	printf("dir_type = %d\n", dir_type);

	if(dir_type < 0){
		printf("[x] Argumento invalido.\n");
		strcpy(str, "Direção inválida.\n");
	}
	else if(p != NULL){

		struct room_door *rd_aux = p->actual_room->doors;

		// Percorre a lista de portas.
		while(rd_aux){
 		
			// Se a porta é na direção pedida.
			if(rd_aux->dir == dir_type){
	
				// TODO porta fechada?

				if(!strcmp(p->actual_room->description, rd_aux->door->room1->description))
					p->actual_room = rd_aux->door->room0;				
				else
					p->actual_room = rd_aux->door->room1;
				
				strcpy(str, "Moveu para a ");
				strcat(str, p->actual_room->description);
				strcat(str, ".\n");
				break;
			}	
        	
			rd_aux = rd_aux->next_door;
		}

		// TODO
		//strcpy(str, "Direção inválida ou porta não existe.\n");
		//printf("[x] Direção inválida ou porta não existe.\n");

    }else{
		printf("[x] Player não encontrdo no servidor.\n");
		strcpy(str, "Player não encontrado no servidor.\n");
	}

	int size = strlen(str);
	memcpy(buffer, str, size);    
    return size;

}

int server_use(int player_id, char *obj_name, char *targ_name, struct player *player_list){

    printf("[v] Requisição do player %d para usar item.\n", player_id);

	int object_id = get_obj_type(obj_name);
	int target_id = get_obj_type(targ_name);

    struct player *p = get_player_by_id(player_list, player_id);
	struct item *i = get_player_item_by_id(p, object_id);

	if(object_id == FLASHLIGHT){
		if(i->state){
			i->state = 0;
		}else{
			i->state = 1;
		}		
	}else if(object_id == KEY){
		// get door by dir
		
		struct room_door *rd_aux = p->actual_room->doors;
		int dir_type = get_dir_type(targ_name);

		// Percorre a lista de portas.
		while(rd_aux){
 		
			// Se a porta é na direção pedida.
			if(rd_aux->dir == dir_type){
				rd_aux->door->state = OPEN;
				return 0;
			}	
        	
			rd_aux = rd_aux->next_door;
		}

	}else{
		printf("[x] Objeto inválido!\n");
	}	

    return 0;

}

void server_move_response(int sock, unsigned char *data, int data_size, char *dst_ip, struct player *player_list, char *buffer, int buffer_offset){

	struct data_t data_h;
	memcpy(&data_h, data, sizeof(struct data_t));	
	int offset = sizeof(struct data_t);
	int name_size = data_size - sizeof(struct data_t);

	printf("name size = %d\n", name_size);

	char *value = malloc(sizeof(char) * name_size);
	//value[name_size] = '\0';

	memcpy(value, &data[offset], name_size);	
			
	// TODO Troca nomes

	printf("[v] Respondendo requisição de mover.\n");

	//struct Jogador *player = (struct Jogador*)users+data_h.id*sizeof(struct Jogador);
	
	unsigned char *help_data = malloc(300);

	//int payload_size = move(*player, value, help_msg); 	
	int payload_size = server_move(0, value, player_list, buffer);

	int header_size = sizeof(struct data_t);

	memset(help_data, 0, 0);

	struct data_t *data_header = (struct data_t*)(help_data);
	data_header->cmd = data_h.cmd;
	data_header->id = data_h.id;
	data_header->ack = 1;
	data_header->off = 0;

	memcpy(&help_data[header_size], buffer, payload_size);	

	send_udp(sock, dst_ip, help_data, header_size + payload_size, PORT, ITF);		

}

void server_catch_response(int sock, unsigned char *data, int data_size, char *dst_ip, struct player *player_list, char *buffer, int buffer_offset){

	struct data_t data_h;
	memcpy(&data_h, data, sizeof(struct data_t));	
	int offset = sizeof(struct data_t);
	int name_size = data_size - sizeof(struct data_t);

	char *value = malloc(sizeof(char) * name_size);
	//value[name_size] = '\0';

	memcpy(value, &data[offset], name_size);	
		
	printf("[v] Respondendo requisição de examinar.\n");
	

	unsigned char *help_data = malloc(300);

	printf("%s\n", value);

	int payload_size = server_catch(0, value, player_list, buffer);

	//int payload_size = server_examine(0, value, player_list, buffer, buffer_offset);	
	
	int header_size = sizeof(struct data_t);

	memset(help_data, 0, 0);

	struct data_t *data_header = (struct data_t*)(help_data);
	data_header->cmd = data_h.cmd;
	data_header->id = data_h.id;
	data_header->ack = 1;
	data_header->off = 0;

	memcpy(&help_data[header_size], buffer, payload_size);	
	send_udp(sock, dst_ip, help_data, header_size + payload_size, PORT, ITF);		

}

void server_drop_response(int sock, unsigned char *data, int data_size, char *dst_ip, struct player *player_list, char *buffer, int buffer_offset){


	struct data_t data_h;
	memcpy(&data_h, data, sizeof(struct data_t));	
	int offset = sizeof(struct data_t);
	int name_size = data_size - sizeof(struct data_t);

	char *value = malloc(sizeof(char) * name_size);
	//value[name_size] = '\0';

	memcpy(value, &data[offset], name_size);	
		
	printf("[v] Respondendo requisição de examinar.\n");
	
	unsigned char *help_data = malloc(300);

	printf("%s\n", value);

	int payload_size = server_drop(0, value, player_list, buffer);	
	int header_size = sizeof(struct data_t);

	memset(help_data, 0, 0);

	struct data_t *data_header = (struct data_t*)(help_data);
	data_header->cmd = data_h.cmd;
	data_header->id = data_h.id;
	data_header->ack = 1;
	data_header->off = 0;

	memcpy(&help_data[header_size], buffer, payload_size);	
	send_udp(sock, dst_ip, help_data, header_size + payload_size, PORT, ITF);		

}


void server_inventory_response(int sock, int user_id, int cmd, char *dst_ip, struct player *player_list){

	// TODO Troca nomes

	printf("[v] Respondendo requisição de invetorio.\n");

	//struct Jogador *player = (struct Jogador*)users+user_id*sizeof(struct Jogador);
	
	unsigned char *help_msg = malloc(200);
	unsigned char *help_data = malloc(300);

	// int payload_size server_show_inventary(int player_id, struct player *player_list){
 	int payload_size = server_show_inventary(0, player_list, help_msg, 0); 

	printf("%s (size = %d)\n", help_msg, payload_size);

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

void server_help_response(int sock, int cmd, int user_id, char dst_ip[]){
	
	unsigned char *help_msg = malloc(200);
	unsigned char *help_data = malloc(300);

	int payload_size = game_help(help_msg, 0);	
	int header_size = sizeof(struct data_t);

	memset(help_data, 0, 0);

	struct data_t *data_header = (struct data_t*)(help_data);
	data_header->cmd = cmd;
	data_header->id = user_id;
	data_header->ack = 1;
	data_header->off = 0;

	memcpy(&help_data[header_size], help_msg, payload_size);	

	printf("[v] Respondendo requisição de ajuda.\n");

	send_udp(sock, dst_ip, help_data, header_size + payload_size, PORT, ITF);		
	
}

void server_examine_response(int sock, unsigned char *data, int data_size, char *dst_ip,  struct player *player_list, char *buffer, int buffer_offset){

	struct data_t data_h;
	memcpy(&data_h, data, sizeof(struct data_t));	
	int offset = sizeof(struct data_t);
	int name_size = data_size - sizeof(struct data_t);

	char *value = malloc(sizeof(char) * name_size);
	//value[name_size] = '\0';

	memcpy(value, &data[offset], name_size);	
		
	printf("[v] Respondendo requisição de examinar.\n");
	
	unsigned char *help_data = malloc(300);

	int payload_size = server_examine(0, value, player_list, buffer, buffer_offset);	
	
	int header_size = sizeof(struct data_t);

	memset(help_data, 0, 0);

	struct data_t *data_header = (struct data_t*)(help_data);
	data_header->cmd = data_h.cmd;
	data_header->id = data_h.id;
	data_header->ack = 1;
	data_header->off = 0;

	memcpy(&help_data[header_size], buffer, payload_size);	
	send_udp(sock, dst_ip, help_data, header_size + payload_size, PORT, ITF);		

}
	

int main(){

    // Game layer

    printf("------------- Simple RPG -------------- \n");

    struct item *fl0 = malloc(sizeof(struct item)); // Aloca uma lanterna.
    fl0->type = FLASHLIGHT;
    fl0->id = 0;
	fl0->state = 0;

    struct item *k0 = malloc(sizeof(struct item)); // Aloca uma chave.
    k0->type = KEY;
    k0->id = 1;
	k0->state = -1;

    struct item *mp0 = malloc(sizeof(struct item)); // Aloca um mapa.
    mp0->type = MAP;
    mp0->id = 2;
	mp0->id = -1;

    struct room *rm_a = NULL;   // Aloca uma sala.
    init_room(&rm_a, LIGHT_ON, "Sala A");
    add_room_item(rm_a, mp0);
    add_room_item(rm_a, k0); 
    add_room_item(rm_a, fl0);

    // Lista objetos na sala.
    //char *buffer = malloc(100);
    //to_string_room_itens(rm_a, buffer, 0);
    //printf("%s", buffer);

    // Remove um objeto.
    //remove_room_item(rm_a, k0); 

    // Lista novamente os objetos.
    //memset(buffer, 0, 100);
    //to_string_room_itens(rm_a, buffer, 0);
    //printf("%s", buffer);

    struct room *rm_b = NULL; // Aloca mais uma sala.
    init_room(&rm_b, LIGHT_ON, "Sala B");
    add_room_item(rm_b, fl0);

    init_door(rm_a, NORTH, rm_b, SOUTH, OPEN); // Inicializa porta aberta.    

    struct room *rm_c = NULL; // Aloca mais uma sala.
    init_room(&rm_c, LIGHT_OFF, "Sala C");
    
    init_door(rm_b, EAST, rm_c, WEST, CLOSE); // Inicializa porta aberta.    

    struct player *p = NULL;
    init_player(&p, 0, "Player A", rm_a);    

    printf("Select the operation mode: server test(t) or server run(r)");
    int c = getchar();

    if(c == 't'){

        char *input = malloc(sizeof(char) *50);
        char *cmd = malloc(sizeof(char)*50); 
        // Server receiver simulation - Server layer
        while(1){

            char *param1 = malloc(sizeof(char) *50); 
            char *param2 = malloc(sizeof(char) *50);   

            printf("- Comando:~$\n"); 
            fgets(input, 50, stdin);
            sscanf(input, "%s %s %s", cmd, param1, param2);  

			char *buffer = malloc(300*sizeof(char)); 

            switch(get_cmd_type(cmd)){
                case EXAMINE:
                    server_examine(0, param1, p, buffer, 0);
					printf("%s\n", buffer);
                    break;
                case INVENTORY:
                    server_show_inventary(0, p, buffer, 0);
					printf("%s\n", buffer);
                    break;
                case CATCH:
                    server_catch(0, param1, p, buffer);
					printf("%s\n", buffer);
                    break;
                case DROP:
                    server_drop(0, param1, p, buffer);
					printf("%s\n", buffer);
                    break;
                case MOVE:
                    server_move(0, param1, p, buffer);
                    break;
				case USE:
					server_use(0, param1, param2, p);
                case HELP:
                    break;
                default:
                    printf("Comando desconhecido.\n");
            }

        }

    }else{
 
	    int ssock, rsock, data_size;
    
        unsigned char *data;
  
 		int on;
   		struct ifreq ifr;

        // Abrindo socket para recebimento de dados.
        printf("[v] Inicializando sockets.\n");
   		// ETH_P_IP
   		if((rsock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0) {	
            printf("Erro na criacao do socket.\n");
            exit(1);
        }

		strcpy(ifr.ifr_name, "lo");
		if(ioctl(rsock, SIOCGIFINDEX, &ifr) < 0)
			printf("erro no ioctl!");
		ioctl(rsock, SIOCGIFFLAGS, &ifr);
		ifr.ifr_flags |= IFF_PROMISC;
		ioctl(rsock, SIOCSIFFLAGS, &ifr);


        // Abrindo socket para envio de dados.
        if((ssock = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW)) < 0)
            printf("[x] Erro na abertura do socket.\n");

        // Inicializa buffer para armazenar IP e MAC dos usuários.
        printf("[v] Inicializa buffer de usuários.\n");
        //unsigned char *users = malloc(5*sizeof(struct Jogador));
        printf("[v] Inicializa buffers de endereços de usuário.\n");
        unsigned char *users_info = malloc(5 *(18 + 6));

        // Alocando e inicializando buffer de dados.	
        data = (unsigned char*)malloc(300); 

        while(1){

            int offset = 0;

            // Limpa buffer de dados.
            // TODO Usar tamanho máximo fixo do buffer? Eis a questão.
            memset(data, 0, 300);
            
            printf("[v] Aguardando requisição...\n");
            // Pega um pacote qualquer endereçado ao servidor.	
            
            unsigned char *user_mac = malloc(6);
            char *user_ip = malloc(16);

            if((data_size = get_packet(rsock, PORT, ITF, -1, data, user_ip, user_mac)) > 0){

                printf("[?] Servidor recebeu uma nova requisição.\n");
                printf("[?] MAC:");
                print_mac(user_mac);
                printf("  IP: %s\n", user_ip);

                struct data_t data_h;
                memcpy(&data_h, data, sizeof(struct data_t));	
                //offset += sizeof(struct data_t);

				char *buffer = malloc(400*sizeof(char));

                // Requisição de login.
                //if(data_h.cmd == LOGIN){
                  //  printf("[v] Servidor pediu um pedido de login.\n");	

                //}

                // Se não é uma requisição de login, é necesário verifica se o usuário ja está logado, verificando IP, MAC e ID do usuário. 
        
                // TODO Verifica se já está logado.

                // User OK, segue o fluxo dos comandos de jogo.

                switch(data_h.cmd){
                    case EXAMINE:
                    	server_examine_response(ssock, data, data_size, user_ip, p, buffer, 0);
						break;
                    case MOVE:
						server_move_response(ssock, data, data_size, user_ip, p, buffer, 0);
                        break;	
                    case CATCH:
						server_catch_response(ssock, data, data_size, user_ip, p, buffer, 0);
                        break;
                    case DROP:
						server_drop_response(ssock, data, data_size, user_ip, p, buffer, 0);
                        break;
                    case INVENTORY:
						server_inventory_response(ssock, data_h.id, data_h.cmd, user_ip, p);
						break;
                    case USE:
						// TODO
                        break;
                    //case SPEAK:
                    //    break;
                    //case WHISPER:
                        //send_whisper(sock, data, name_size, data_h.id, data_h.cmd, user_ip, data, users);	
                        break;
                    case HELP:
                        server_help_response(ssock, data_h.cmd, data_h.id, user_ip);
                        break;
                    default:
                        printf("[x] Solicitação com código desconhecido\n");
                }

            }
        }

        printf("[v] Fechando sokets.\n");
        close(ssock);
        close(rsock);    
    
    }
    return 0;

}


