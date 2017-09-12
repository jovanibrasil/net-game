
#ifndef _GAME_
#define _GAME_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "gametypes.h"

/*
 *
 * Métodos auxiliares
 *
 */

const char* description(int type);
char* lower_case(char cmd[]);
int get_cmd_type(char cmd[]);    
int get_obj_type(char cmd[]);
int get_dir_type(char cmd[]);
const char* get_dir_str(int dir_type);
void init_room(struct room **rm, int light_state, char dscp[16]);
void add_room_item(struct room *rm, struct item *it); 
void remove_player_item(struct player *p, struct item *it);
int to_string_player_itens(struct player *p, char *buffer, int buffer_offset);
void add_room_door();

/*
 *
 * Player methods
 *
 * */

void init_player(struct player **p, int id, char name[16], struct room *actual_room);
void add_player_item(struct player *p, struct item *it);
void remove_room_item(struct room *rm, struct item *it);
int to_string_room_itens(struct room *rm, char *buffer, int buffer_offset);   

/*
 *
 * Door Methods.
 *
 */

void add_door(struct room  *r, struct room_door *rd);
void init_door(struct room  *ra, int pos_a, struct room *rb, int pos_b, int state);    
int to_string_room_doors(struct room *rm, char *buffer, int buffer_offset);   

/*
 *
 * Game functions implementation.
 *
 * Here we have only the game layer.
 *
 */

int game_examine_room(struct player *p, struct game *game);
int game_examine_item(struct item *it);
int game_show_inventary(struct player *p, char *buffer);
int game_catch(struct player *p, struct item *it);
int game_drop(struct player *p, struct item *it);
int game_move();

/*
 * Server layer functions implementation. 
 *
 */

struct player* get_player_by_id(struct player *p, int player_id);
struct item* get_room_item_by_type(struct room *r, int object_id);
struct item* get_player_item_by_id(struct player *p, int object_id);

#endif
