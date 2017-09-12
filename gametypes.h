#ifndef _GAMETYPES_
#define _GAMETYPES_


/*
 * Tipos de objetos e definições.
 *
 *
 *
 *
 */

#define MAP 0
#define FLASHLIGHT 1
#define KEY 2

#define INVALID_PLAYER		-1
#define INVALID_ARG			-2

// Estados de objetos.

#define LIGHT_ON 100
#define LIGHT_OFF -100

// Ações do player.

#define EXAMINE     0x00
#define CATCH       0x01
#define DROP        0x02
#define MOVE        0x03
#define INVENTORY   0x04
#define HELP	    0x05

#define ROOM 		54
#define NORTH   	55
#define SOUTH   	45
#define EAST    	46
#define WEST    	47

#define OPEN 		60
#define CLOSE  		61

struct item {

    int type;  // Tipo do Item.
    int id;

};

struct inventory_item {

    struct item *my_item; // Ponteiro para o item em questão.
    struct inventory_item *next_item; // Ponteiro para o próximo item do inventário.

}; 

struct door {

    int state; // Estado da porta, aberta ou fechada.
    struct room *room0; // Ponteiros para as salas que a porta liga.
    struct room *room1;
    char description[16];

};

struct room_door {
   
	int dir;	
    struct door *door; // Ponteiro para a porta.
    struct room_door *next_door; // Ponteiro para a próxima porta da sala.
    int pos;

};

struct room {

    struct inventory_item *inventory; // Lista de itens disponíveis na sala.
    struct room_door *doors;    // Lista de portas disponíveis.
    int light_state; // Iluminação acesa ou apagada. 
    char description[16]; // Descrição.
    struct room *next_room;

};

struct player {

    struct inventory_item *inventory; // Ponteiro para o inventário. 
    struct room *actual_room; // Sala atual do jogador.
    struct player *next_player;
    char name[20];
    int id;

};

struct game {

    int players_counter; // Contador de players.
    struct room *room_lsthdr; // Lista de salas.
    struct player *player_lsthdr; // Lista de jogadores. 

};

#endif
