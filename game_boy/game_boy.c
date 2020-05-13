/*
 * main.c
 *
 *  Created on: 28 feb. 2019
 *      Author: utnso
 */

#include "game_boy.h"

int main(int argc, char** argv){

	if(argc < 4){
		printf("Parámetros faltantes\n");
		exit(EXIT_FAILURE);
	}

	char* destinatario = argv[1];

	if(string_equals_ignore_case(destinatario, "TEAM")){

	} else if(string_equals_ignore_case(destinatario, "BROKER")) {

	} else if(string_equals_ignore_case(destinatario, "GAMECARD")){

	} else if(string_equals_ignore_case(destinatario, "SUSCRIPTOR")){

	} else {
		printf("%s no es un proceso válido\n", destinatario);
		exit(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}
