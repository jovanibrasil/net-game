/*
 * Autor: Jovani Brasil.
 *
 */

#include "gameutils.h"
#include "gametypes.h"

#include "stdio.h"
#include "ctype.h"
#include "string.h"

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
