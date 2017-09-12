/*
 *
 * 
 *
 *
 * Considera-se que podem existir n objetos iguais em uma sala,
 * não diferindo em relação aos seus atributos. 
 *
 * It0 - funcionando básico
 *
 * printf("Examinar [sala/objeto]\n");
 * printf("Mover [N/S/L/O]\n");
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
int server_examine(int player_id, char *object_name, struct player *player_list){

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
	
				char *buffer = malloc(sizeof(char)*400);
						
				int len = strlen(p->actual_room->description);  

				memcpy(buffer, p->actual_room->description, len);	
		
				int offset = to_string_room_itens(p->actual_room, buffer+len, 0) + len;
								
				to_string_room_doors(p->actual_room, buffer, offset);

				printf("%s\n", buffer);

				return 1;
			}else{
				
				printf("[v] Requisição para examinar objeto %d.\n", obj_type);

				// Busca objeto na sala em que o player está.
				struct item *it = get_room_item_by_type(p->actual_room, obj_type);  
			
				if(it){
					printf("     $ %s\n", description(it->type));
					return 1;
			 	}

				return 1;
			}

		}else{
			printf("[x] Jogador não encontrado.");
			// Player não encontrado.
			return INVALID_ARG; 
		}
	}
		

}

int server_show_inventary(int player_id, struct player *player_list){
   
    printf("Requisição de inventario do player %d.\n", player_id);

    char *buffer = malloc(sizeof(char)*100); 
    
    struct player *p = get_player_by_id(player_list, player_id);
        
    if(p != NULL){
        game_show_inventary(p, buffer); 
        printf("%s\n", buffer);
        return 1;
    }

    return 0;

}

int server_catch(int player_id, char *object_name, struct player *player_list){

    printf("Requisição do player %d para coleta de item.\n", player_id);

	int object_id = get_obj_type(object_name);

    char *buffer = malloc(sizeof(char)*100); 
    struct player *p = get_player_by_id(player_list, player_id);
     
    if(p != NULL){
        struct item *it = get_room_item_by_type(p->actual_room, object_id);  
        if(it){
            game_catch(p, it);
            return 1;
        }
    }

    return 0;

}

int server_drop(int player_id, char *object_name, struct player *players_list){
 
    printf("[v] Requisição do player %d para largar item.\n", player_id);

	int object_id = get_obj_type(object_name);

    char *buffer = malloc(sizeof(char)*100); 
    struct player *p = get_player_by_id(players_list, player_id);
    
    if(p != NULL){
        struct item *it = get_player_item_by_id(p, object_id); 
        if(it){
            game_drop(p, it);
            return 1;
        }else{
			printf("[x] Item selecionado não encontrado pelo servidor!\n");
		}
    }else{
		printf("[x] Player não encontrado pelo servidor!\n");
	}

    return 0;

}

int server_move(int player_id, char dir_str[20], struct player *players_list){
 
    printf("[v] Requisição do player %d se mover.\n", player_id);

    char *buffer = malloc(sizeof(char)*100); 
 
	int dir_type = get_dir_type(dir_str);

	if(dir_type < 0){
		printf("arg invalido\n");
		return INVALID_ARG;
	}

	struct player *p = get_player_by_id(players_list, player_id);

    if(p != NULL){

		struct room_door *rd_aux = p->actual_room->doors;

		// Percorre a lista de portas.
		while(rd_aux){
 		
			// Se a porta é na direção pedida.
			if(rd_aux->dir == dir_type){
	
				//printf("sala atual --> %s\n", p->actual_room->description);	
				//printf("room0 da porta--> %s\n", rd_aux->door->room0->description);	
				//printf("room1 da porta--> %s\n", rd_aux->door->room1->description);	

				if(!strcmp(p->actual_room->description, rd_aux->door->room1->description)){
					//printf("escolher room0\n");
					p->actual_room = rd_aux->door->room0;
				} else {
					//printf("escolheu room1\n");
					p->actual_room = rd_aux->door->room1;
				}
			
				//printf("--> %s\n", p->actual_room->description);	
				//printf("--> %s\n", rd_aux->door->room1->description);	

				rd_aux = p->actual_room->doors;

				//printf("sala atual --> %s\n", p->actual_room->description);	
				//printf("room0 da porta--> %s\n", rd_aux->door->room0->description);	
				//printf("room1 da porta--> %s\n", rd_aux->door->room1->description);	

				//return 1;
			}	
        	
			rd_aux = rd_aux->next_door;
		}
    }else{
		printf("Inválido\n");
	}

    return 0;

}

int server_help(int player_id){
	
	printf("Requisição do player por ajuda.\n");

	printf("Examinar [sala/objeto]\n");
	printf("Mover [N/S/L/O]\n");
	printf("Pegar [objeto]\n");
	printf("Largar [objeto]\n");
	printf("Inventário\n");
	printf("Usar [objeto] {alvo}\n");
	printf("Falar [texto]\n");
	printf("Cochichar [texto] [jogador]\n");
	printf("Ajuda\n");	

}

int main(){

    // Game layer

    printf("------------- MUD-GAME TEST -------------- \n");

    struct item *fl0 = malloc(sizeof(struct item)); // Aloca uma lanterna.
    fl0->type = FLASHLIGHT;
    fl0->id = 0;

    struct item *k0 = malloc(sizeof(struct item)); // Aloca uma chave.
    k0->type = KEY;
    k0->id = 1;

    struct item *mp0 = malloc(sizeof(struct item)); // Aloca um mapa.
    mp0->type = MAP;
    mp0->id = 2;

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
    init_room(&rm_c, LIGHT_ON, "Sala C");
    
    init_door(rm_b, EAST, rm_c, WEST, OPEN); // Inicializa porta aberta.    

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

            switch(get_cmd_type(cmd)){
                case EXAMINE:
                    server_examine(0, param1, p);
                    break;
                case INVENTORY:
                    server_show_inventary(0, p);
                    break;
                case CATCH:
                    server_catch(0, param1, p);
                    break;
                case DROP:
                    server_drop(0, param1, p);
                    break;
                case MOVE:
                    server_move(0, param1, p);
                    break;
                case HELP:
                    server_help(0);
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

                // Requisição de login.
                //if(data_h.cmd == LOGIN){
                  //  printf("[v] Servidor pediu um pedido de login.\n");	

                //}

                // Se não é uma requisição de login, é necesário verifica se o usuário ja está logado, verificando IP, MAC e ID do usuário. 
        
                // TODO Verifica se já está logado.

                // User OK, segue o fluxo dos comandos de jogo.

                switch(data_h.cmd){
                    case EXAMINE:
                        //send_examine(sock, data, data_size, user_ip, users);	
                        break;
                    case MOVE:
                        //send_move(sock, data, data_size, user_ip, users);
                        break;	
                    case CATCH:
                        break;
                    case DROP:
                        break;
                    case INVENTORY:
                        //send_inventory(sock, data_h.id, data_h.cmd, user_ip, users);
                        break;
                    //case USE:
                    //    break;
                    //case SPEAK:
                    //    break;
                    //case WHISPER:
                        //send_whisper(sock, data, name_size, data_h.id, data_h.cmd, user_ip, data, users);	
                        break;
                    case HELP:
                        //send_help(sock, data_h.cmd, data_h.id, user_ip);
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


