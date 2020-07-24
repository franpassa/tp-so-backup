#include "gamecard.h"

t_config* get_config(char* config_path){
	t_config* mi_config = config_create(config_path);
	if(mi_config == NULL){
		terminar_aplicacion("Error abriendo el archivo de configuración\n");
	}
	return mi_config;
}

t_log* crear_log(char* log_path){
	t_log* mi_log = log_create(log_path, "Gamecard", true, LOG_LEVEL_INFO);
	if(mi_log == NULL){
		terminar_aplicacion("Error creando el log\n");
	}
		return mi_log;
}

void esperar_tiempo_retardo(){
	int tiempo_retardo = config_get_int_value(config, "TIEMPO_RETARDO_OPERACION");
	sleep(tiempo_retardo);
}

void terminar_aplicacion(char* mensaje){
	char* msg_error = string_new();
	string_append_with_format(&msg_error, "ERROR: %s", mensaje);
	log_error(logger, msg_error);
	free(msg_error);

	abort();
}

bool id_en_lista(t_list* lista, uint32_t id){

	bool id_igual(void* id_en_lista){
		uint32_t* id_casteado = (uint32_t*) id_en_lista;
		return id == *id_casteado;
	}

	return list_any_satisfy(lista, id_igual);
}

void agregar_id(t_list* lista, uint32_t id){
	uint32_t* id_a_lista = malloc(sizeof(uint32_t));
	*id_a_lista = id;

	list_add(lista, (void*) id_a_lista);
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
	if(list_is_empty(list)) return string_duplicate("[]");

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
			if(substring[0] == '\n') {
				char* substring_mod = string_substring_from(substring, 1);
				free(substring);
				substring = substring_mod;
			}
			list_add(list_strings, substring);
			inicio_substring += largo_substring;
			largo_substring = min(tamanio, largo_string - inicio_substring);
		}
	}
	return list_strings;
}

void free_array(char** array){
	int index = 0;
	char* str;
	while((str = array[index]) != NULL){
		free(str);
		index++;
	}
	free(array);
}

t_info_msg* init_info_msg(queue_name tipo, void* msg, uint32_t id){
	t_info_msg* info_msg = malloc(sizeof(t_info_msg));

	info_msg->tipo_msg = tipo;
	info_msg->msg = msg;
	info_msg->id_msg = id;

	return info_msg;
}

void free_info_msg(t_info_msg* info){
	free(info->msg);
	free(info);
}



