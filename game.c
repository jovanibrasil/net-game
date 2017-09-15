/*
 * Autor: Jovani Brasil.
 *
 */


#include "game.h"
#include "gameutils.h"

/*
 *
 * Room methods
 *
 */

void init_room(struct room **rm, int light_state, char dscp[16]){

    printf("Incializando %s.\n", dscp);

    (*rm) = (struct room*)malloc(sizeof(struct room)); // Aloca a sala.
    (*rm)->inventory = NULL;
    (*rm)->light_state = light_state;
    strcpy((*rm)->description, dscp); ;

}

void add_room_item(struct room *rm, struct item *it){

    printf("Adicionando item %s no inventario de %s.\n", name(it->type), rm->description);

    struct inventory_item *iit = malloc(sizeof(struct inventory_item)); // Aloca mais um item de inventário.
    iit->my_item = it;
    iit->next_item = NULL;

    if(rm->inventory == NULL){ 
        rm->inventory = iit;
    }else{ 
        iit->next_item = rm->inventory;
        rm->inventory = iit;
    }

}

void remove_player_item(struct player *p, struct item *it){

    printf("Removendo item %s no inventario de %s.\n", name(it->type), p->name);

    struct inventory_item *iit = p->inventory;
    
    // Testa o primeiro item.
    if(iit->my_item->type == it->type){   
            p->inventory = iit->next_item;
            return;
    }

    // Vai testando o próximo.
    while(iit->next_item != NULL){
        if(iit->next_item->my_item->type == it->type){    
            iit->next_item = iit->next_item->next_item;
            return;
        } 
    }
   
}

int to_string_player_itens(struct player *p, char *buffer, int buffer_offset){
   
    struct inventory_item *iit = p->inventory;
    
    char str[100];

   	if(iit == NULL){
		strcpy(str, "Inventário vazio!\n");
	}else{
		strcpy(str, "Meus itens:\n");
	}

    while(iit != NULL){
        strcat(str, name(iit->my_item->type));
        strcat(str, "\n"); 
        iit = iit->next_item;
    }

    int size = strlen(str);
    memcpy(buffer, str, size);    

    return size;

}

void add_room_door(){

    //rm->room_door = 
    


}

/*
 *
 * Player methods
 *
 * */

void init_player(struct player **p, int id, char name[16], struct room *actual_room){

    printf("[v] Incializando %s.\n", name);

    (*p) = (struct player*)malloc(sizeof(struct player)); // Aloca a sala.
    (*p)->inventory = NULL;
    (*p)->id = id;
    (*p)->actual_room = actual_room;
    (*p)->stage = STAGE0;
	strcpy((*p)->name, name); ;

}

void add_player_item(struct player *p, struct item *it){

    printf("[v] Adicionando item %s no inventario da %s.\n", description(it->type), p->name);

    struct inventory_item *iit = malloc(sizeof(struct inventory_item)); // Aloca mais um item de inventário.
    iit->my_item = it;
    iit->next_item = NULL;

    if(p->inventory == NULL){ 
        p->inventory = iit;
    }else{ 
        iit->next_item = p->inventory;
        p->inventory = iit;
    }

}

void remove_room_item(struct room *rm, struct item *it){

    printf("[v] Removendo item %s no inventario da %s.\n", description(it->type), rm->description);

    struct inventory_item *iit = rm->inventory;
    
    // Testa o primeiro item.
    if(iit->my_item->type == it->type){   
            rm->inventory = iit->next_item;
            return;
    }

    // Vai testando o próximo.
    while(iit->next_item != NULL){
        if(iit->next_item->my_item->type == it->type){    
            iit->next_item = iit->next_item->next_item;
            return;
        }
	    iit = iit->next_item;	
    }


}

int to_string_room_itens(struct room *rm, char *buffer, int offset){
   
    struct inventory_item *iit = rm->inventory;
    
    char str[100];

	if(iit == NULL){
		strcpy(str, "\nNão existem itens na sala!\n");
	}else{
		strcpy(str, "\nItens na sala:\n");
	}

    while(iit != NULL){
		strcat(str, "	");
        strcat(str, name(iit->my_item->type));
        strcat(str, "\n"); 
        iit = iit->next_item;

    }

    int size = strlen(str);
	memcpy(buffer, str, size);    
    return size;

}

/*
 *
 * Door Methods.
 *
 */


void add_door(struct room  *r, struct room_door *rd){

    struct room_door *rd_aux = r->doors;

    if(rd_aux == NULL){
        rd->next_door = NULL;
        r->doors = rd;
    }else{
        
        rd->next_door = rd_aux;
        r->doors = rd; 
    }
        
} 

void init_door(struct room  *ra, int dir_a, struct room *rb, int dir_b, int state, int is_end){

    printf("[v] Inicializando porta entre %s e %s.\n", ra->description, rb->description);

    struct door *d = malloc(sizeof(struct door));

    d->state = state;
    d->room0 = ra;
    d->room1 = rb;
	d->is_end = is_end;

    struct room_door *rd_a = malloc(sizeof(struct room_door)); 
    rd_a->door = d;
	rd_a->dir = dir_a;

	struct room_door *rd_b = malloc(sizeof(struct room_door)); 
    rd_b->door = d;
	rd_b->dir = dir_b;

    add_door(ra, rd_a);
    add_door(rb, rd_b);
   
}    

int to_string_room_doors(struct room *rm, char *buffer, int buffer_offset){

	struct room_door *rd_aux = rm->doors;

 	char str[100];

    strcpy(str, "Portas da sala:\n");

	while(rd_aux){
 
        strcat(str, "	Porta ");
		strcat(str, get_dir_str(rd_aux->dir));
		strcat(str, "(");
		strcat(str, door_state(rd_aux->door->state));	
		strcat(str, ")");	
		strcat(str, "\n");  

		rd_aux = rd_aux->next_door;
	}

	int size = strlen(str);
    memcpy(&buffer[buffer_offset], str, size);    

	return size + buffer_offset;

}

/*
 *
 * Game functions implementation.
 *
 */

int game_examine_room(struct player *p, struct game *game, char *buffer){
    
	char str[100];	

	// Busca por item lanterna no inventario do jogador
	struct item *it = get_player_item_by_id(p, FLASHLIGHT);

	// Se não há luz e lanterna está desligada
	if(p->actual_room->light_state == LIGHT_OFF){
		if(it == NULL){
    		strcpy(str, "Não há condição de examinar a sala. Não há luz.\n");
			int size = strlen(str);
    		memcpy(buffer, str, size);    
			return size;
		}
		else if(it->state == LIGHT_OFF){
			strcpy(str ,"Não há condição de examinar a sala. Não há luz.\n");
			int size = strlen(str);
    		memcpy(buffer, str, size);    
			return size;
		}
	}

	int len = strlen(p->actual_room->description);  
	memcpy(buffer, p->actual_room->description, len);		
	int offset = to_string_room_itens(p->actual_room, buffer+len, 0) + len;
							
	return to_string_room_doors(p->actual_room, buffer, offset);

}

int game_examine_item(struct player *p, int obj_type, char *buffer){

		char str[300];

		// Busca objeto na sala em que o player está.
		struct item *it = get_room_item_by_type(p->actual_room, obj_type);  
	
		// Objeto também pode estar no inventário.
		if(!it)
			it = get_player_item_by_id(p, obj_type);

		if(it){
			strcpy(str, description(it->type));
			
			if(it->type == MAP){
				strcat(str, get_map(p->stage));
				strcat(str, "Você está na sala ");
				strcat(str, p->actual_room->description);
				strcat(str, ".\n");
			}
			
			}else{
			strcpy(str, "Objeto não foi encontrado.\n");
		}

		int size = strlen(str);
		memcpy(buffer, str, size);    
		return size;

}

int game_show_inventary(struct player *p, char *buffer){
    return to_string_player_itens(p, buffer, 0);
}

int game_catch(struct player *p, struct item *it){
    add_player_item(p, it);
    remove_room_item(p->actual_room, it);
}

int game_drop(struct player *p, struct item *it){
    remove_player_item(p, it);
    add_room_item(p->actual_room, it);
}

/*
 * Server layer functions implementation. 
 *
 */

struct player* get_player_by_id(struct player *p, int player_id){

	printf("[v] Buscando player %d.\n", player_id);

    while(p){
        if(p->id == player_id){
            return p;
        }    
        p = p->next_player;
    }

    return NULL;

}

struct item* get_room_item_by_type(struct room *r, int object_type){

    printf("[v] Buscando item do tipo %d na sala %s.\n", object_type, r->description);

    struct inventory_item *iit = r->inventory;

    while(iit != NULL){
        if(iit->my_item->type == object_type){
            return iit->my_item;
        }    
        iit = iit->next_item;
    }

    return NULL;

}

struct item* get_player_item_by_id(struct player *p, int object_id){

    printf("[v] Buscando item do tipo %d no inventorio do player %s.\n", object_id, p->name);

    struct inventory_item *iit = p->inventory;

    while(iit != NULL){
        //printf("%d  ==  %d ?\n", iit->my_item->id, object_id);	
		if(iit->my_item->type == object_id){
            return iit->my_item;
        }    
        iit = iit->next_item;
    }

    return NULL;

}

int game_help(char *buffer, int buffer_offset){

    printf("[v] Requisição do player por ajuda.\n");

    char str[200];

	strcpy(str, "Comandos:.\n");

	strcat(str, "Examinar [sala/objeto]\n");
	strcat(str, "Mover [N/S/L/O]\n");
	strcat(str, "Pegar [objeto]\n");
	strcat(str, "Largar [objeto]\n");
	strcat(str, "Inventário\n");
	strcat(str, "Usar [objeto] {alvo}\n");
	strcat(str, "Ajuda\n");	

    int size = strlen(str);
    memcpy(buffer, str, size);    

    return size;

}

