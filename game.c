#include "game.h"


const char* stage_intro(int stage){

    switch(stage){
        case STAGE0:
            return "STAGE 0 \n	Você deu mole e agora está perdido dentro de um castelo. Tente se salvar, mas cuidado, esse castelo guarda muitas supresas. A saída deste nível fica na porta leste da sala C.\n";
        case STAGE1:
            return "STAGE 1 \n	Saia deste nível. A saída fica na porta norte da sala E.\n";
		default:
			return "[x] Identificador da fase inválido.";
    }

}

const char* stage_final(int stage){

    switch(stage){
        case STAGE0:
            return "Muito bem! \n	Este nível foi fácil. Vamos ver como você se sai no proximo.\n Digite proximo para seguir para o próximo nível.";
        case STAGE1:
            return "Muito bem! \n	Você saiu do castelo. Na verdade você se deu bem pois não tive tempo de fazer mais fases. Digite proximo para voltar ao inicio.";
		default:
			return "[x] Identificador de fase inválido.";
    }

}

const char* get_map(int stage){

    switch(stage){
        case STAGE0:
            return "\n----- ------- -----\n| D |=|  B  |=| C |\n----- ------- -----\n        | |\n       -----\n       | A |\n       -----\n";

        case STAGE1:
            return "\n-----\n| E |\n-----\n  ||\n----- ------- -----\n| D |=|  B  |=| C |\n----- ------- -----\n        | |\n       -----\n       | A |\n       -----\n";
		default:
			return "[x] Identificador de fase inválido.";
    }

}

const char* description(int type){

    switch(type){
        case MAP:
            return "Mapa da saída secreta.";
        case FLASHLIGHT:
            return "Lanterna com bateria infinita.";
        case KEY: 
            return "Chave secreta multiuso.";
		default:
			return "[x] Tipo de objeto inválido.";
    }

}

const char* name(int type){

    switch(type){
        case MAP:
            return "Mapa";
        case FLASHLIGHT:
            return "Lanterna";
        case KEY: 
            return "Chave";
		default:
			return "[x] Tipo de objeto inválido.";
    }

}

const char* get_dir_str(int dir){

    switch(dir){
        case NORTH:
            return "Norte";
        case SOUTH:
            return "Sul";
        case EAST: 
            return "Leste";
		case WEST:
			return "Oeste";
		default:
			return "[x] Direção inválida.";
    }

}

int get_dir_type(char cmd[]){

	char *cmd_aux = lower_case(cmd);

    if(!strcmp(cmd_aux, "norte")){
        return NORTH;
    }else if(!strcmp(cmd_aux, "n")){
        return NORTH;
    }else if(!strcmp(cmd_aux, "leste")){
        return EAST;
    }else if(!strcmp(cmd_aux, "l")){
        return EAST;
    }else if(!strcmp(cmd_aux, "sul")){
        return SOUTH;
    }else if(!strcmp(cmd_aux, "s")){
    	return SOUTH;
	}else if(!strcmp(cmd_aux, "oeste")){
        return WEST;
    }else if(!strcmp(cmd_aux, "o")){
    	return WEST;
    }else{
		return INVALID_ARG;
	}	

}

const char* door_state(int state){

    switch(state){
        case OPEN:
            return "Aberto";
        case CLOSE:
            return "Fechado";
		default:
			return "[x] Estado não encontrado.";
    }

}



/*
 *
 * Métodos auxiliares
 *
 */

char* lower_case(char cmd[]){
    // Reduz todos os caracteres para minúsculo.
    int size = strlen(cmd); 
    int i;
    for(i=0; i<size;i++)
        cmd[i] = tolower(cmd[i]);
    return cmd;
}



int get_cmd_type(char cmd[]){
    
    char *cmd_aux = lower_case(cmd);

    if(!strcmp(cmd_aux, "examinar")){
        return EXAMINE;
    }else if(!strcmp(cmd_aux, "pegar")){
        return CATCH;
    }else if(!strcmp(cmd_aux, "largar")){
        return DROP;
    }else if(!strcmp(cmd_aux, "mover")){
        return MOVE;
    }else if(!strcmp(cmd_aux, "inventario")){
        return INVENTORY;
    }else if(!strcmp(cmd_aux, "ajuda")){
    	return HELP;
	}else if(!strcmp(cmd_aux, "usar")){
		return USE;
	}else if(!strcmp(cmd_aux, "proximo")){
    	return NEXT;
	}else if(!strcmp(cmd_aux, "login")){
		return LOGIN;
	}else{
		return INVALID_ARG;
	}	

}

int get_obj_type(char cmd[]){

    char *cmd_aux = lower_case(cmd);

	printf("[v] cmd_param lowered = %s.\n", cmd_aux);

    if(!strcmp(cmd_aux, "mapa")){
        return MAP;
    }else if(!strcmp(cmd_aux, "lanterna")){
        return FLASHLIGHT;
    }else if(!strcmp(cmd_aux, "chave")){
        return KEY;
	}else if(!strcmp(cmd_aux, "sala")){
		return ROOM;
    }else{
		return INVALID_ARG;		
	}

}

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
 
        strcat(str, "Porta ");
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
 * Here we have only the game layer.
 *
 */

int game_examine_room(struct player *p, struct game *game){
    char *buffer = malloc(200*sizeof(char));
    
    /// Show room itens.
    to_string_room_itens(p->actual_room, buffer, 0);

    // TODO Show online player in the same room.

    

}

int game_examine_item(struct item *it){
	// TODO
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

int game_move(){
   // TODO 
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
	strcat(str, "Falar [texto]\n");
	strcat(str, "Cochichar [texto] [jogador]\n");
	strcat(str, "Ajuda\n");	

    int size = strlen(str);
    memcpy(buffer, str, size);    

    return size;

}

