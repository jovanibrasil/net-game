/*
 * Autor: Jovani Brasil.
 *
 */

#ifndef _GAMEUTILS_
#define _GAMEUTILS_

const char* stage_intro(int stage);
const char* stage_final(int stage);
const char* get_map(int stage);
const char* description(int type);
const char* name(int type);
const char* get_dir_str(int dir);
int get_dir_type(char cmd[]);
const char* door_state(int state);
char* lower_case(char cmd[]);
int get_cmd_type(char cmd[]);
int get_obj_type(char cmd[]);

#endif
