#include "gamecard.h"

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
	abort();
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
	char* list_string = string_new();
	string_append_with_format(&list_string, "[%s]", string_trimmed);
	free(string_trimmed);

	return list_string;
}

char* get_file_as_text(char* file_path){
	FILE* f = fopen(file_path, "r");

	int file_size = get_file_size(f);
	char* file_text = malloc(file_size + 1);
	int bytes_read = fread(file_text, file_size, 1, f);
	if(bytes_read == 1){
		fclose(f);
		file_text[file_size] = '\0';

		return file_text;
	} else {
		return NULL;
	}
}

int min(int a, int b){
	return (a < b) ? a : b;
}

t_list* dividir_string_por_tamanio(char* string, int tamanio){
	t_list* list_strings = list_create();
	int largo_string = string_length(string);

	if(largo_string < tamanio){
		list_add(list_strings, string_duplicate(string));
	} else {
		int inicio_substring = 0;
		int largo_substring = tamanio;
		while(largo_string != inicio_substring){
			char* substring = string_substring(string, inicio_substring, largo_substring);
			if(substring[0] == '\n') substring = substring + 1;
			list_add(list_strings, substring);
			inicio_substring += largo_substring;
			largo_substring = min(tamanio, largo_string - inicio_substring);
		}
	}
	return list_strings;
}
