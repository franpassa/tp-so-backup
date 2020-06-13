#include "../utils/game_card.h"

t_config* get_config(char* config_path){
	t_config* mi_config = config_create(config_path);
	if(mi_config == NULL){
		printf("Error abriendo el archivo de configuración\n");
		exit(-1);
	}
	return mi_config;
}

t_log* crear_log(char* log_path){
	t_log* mi_log = log_create(log_path, "Game Card", 0, LOG_LEVEL_INFO);
	if(mi_log == NULL){
		printf("Error creando el log\n");
		exit(-1);
	}
		return mi_log;
}

void terminar_aplicacion(char* mensaje){
	printf("ERROR: %s\n", mensaje);
	exit(-1);
}


void agregar_a_lista(t_list* lista, int nuevo_elemento){
	int* nuevo_ptr = malloc(sizeof(int));
	*nuevo_ptr = nuevo_elemento;
	list_add(lista, (void*) nuevo_ptr);
}

long get_file_size(FILE* file_ptr){
	fseek(file_ptr, 0, SEEK_END);
	long file_size = ftell(file_ptr);
	fseek(file_ptr, 0, SEEK_SET);

	return file_size;
}

char* list_to_string(t_list* list){
	char* string = string_new();

	for(int i = 0; i < list_size(list); i++){
		int* elemento = (int*) list_get(list, i);
		string_append_with_format(&string, "%d,", *elemento);
	}
	char* string_trimmed = string_substring_until(string, string_length(string)-1);
	free(string);
	return string_trimmed;
}
