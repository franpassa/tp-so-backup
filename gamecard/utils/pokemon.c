#include "gamecard.h"

t_coordenada init_coordenada(uint32_t x, uint32_t y, uint32_t cantidad){
	t_coordenada coordenada;
	coordenada.x = x;
	coordenada.y = y;
	coordenada.cantidad = cantidad;

	return coordenada;
}

t_pokemon init_pokemon(char* nombre, uint32_t x, uint32_t y, uint32_t cantidad){
	t_pokemon pokemon;
	string_to_lower(nombre);
	pokemon.nombre = nombre;
	pokemon.posicion = init_coordenada(x, y, cantidad);

	return pokemon;
}

bool existe_pokemon(char* nombre_pokemon){
	char* path_pokemon = get_pokemon_path(nombre_pokemon);

	struct stat st = {0};
	bool pokemon_existe = stat(path_pokemon, &st) != -1;
	free(path_pokemon);

	return pokemon_existe;
}

char* get_pokemon_path(char* nombre_pokemon){
	char* path_pokemon = string_new();
	char* lowered_pokemon = string_duplicate(nombre_pokemon);
	string_to_lower(lowered_pokemon);
	string_append_with_format(&path_pokemon, "%s/%s", fspaths->files_folder, lowered_pokemon);

	free(lowered_pokemon);

	return path_pokemon;
}

char* get_metadata_path(char* nombre_pokemon){
	char* path_metadata = get_pokemon_path(nombre_pokemon);
	string_append(&path_metadata, "/Metadata.bin");

	return path_metadata;
}

void append_blocks(char** blocks, t_list* new_blocks){
	*blocks = string_substring_until(*blocks, string_length(*blocks) - 1); //Saco el ]

	if(string_length(*blocks) > 1){ // Si hay algun elemento agrego una coma.
		string_append(blocks, ",");
	}

	char* bloques_string = list_to_string(new_blocks);
	string_append_with_format(blocks, "%s]", bloques_string);
}

void agregar_bloques_metadata(t_list* nuevos_bloques, char* nombre_pokemon){
	char* metadata_path = get_metadata_path(nombre_pokemon);
	t_config* metadata_file = config_create(metadata_path);

	if(metadata_file == NULL){
		free(metadata_path);
		return;
	}

	char* pokemon_blocks = config_get_string_value(metadata_file, "BLOCKS");
	char* blocks_dup = string_duplicate(pokemon_blocks);
	append_blocks(&blocks_dup, nuevos_bloques);

	config_set_value(metadata_file, "BLOCKS", blocks_dup);
	config_save(metadata_file);
	config_destroy(metadata_file);
	free(blocks_dup);
	free(metadata_path);
}

void get_pokemon_blocks_and_coordenadas(char* nombre_pokemon, t_list** blocks, t_list** coordenadas){
	*blocks = get_pokemon_blocks(nombre_pokemon);
	if(*blocks == NULL) return;

	char* contenido_bloques = get_blocks_content(*blocks);
	*coordenadas = string_to_coordenadas(contenido_bloques);
	free(contenido_bloques);
}

char* get_last(char** array){
	int index = 0;
	while(array[index] != NULL) index++;

	char* elemento = array[index-1];
	return elemento;
}

bool is_file_open(char* nombre_pokemon){
	char* path_metadata = get_pokemon_path(nombre_pokemon);
	string_append(&path_metadata, "/Metadata.bin");
	bool is_open;

	t_config* metadata_config = config_create(path_metadata);
	if(metadata_config != NULL){
		if(config_has_property(metadata_config, "OPEN")){
			char* open_status = config_get_string_value(metadata_config, "OPEN");
			is_open = open_status[0] == 'Y';
		} else {
			is_open = true;
		}
		config_destroy(metadata_config);
	} else {
		is_open = true;
	}
	free(path_metadata);

	return is_open;
}

void set_open_flag(char* nombre_pokemon, bool value){
	char* path_metadata = get_metadata_path(nombre_pokemon);

	t_config* metadata = config_create(path_metadata);
	char* open_flag;

	if(value){
		open_flag = string_duplicate("Y");
	} else {
		open_flag = string_duplicate("N");
	}

	config_set_value(metadata, "OPEN", open_flag);
	config_save(metadata);

	free(open_flag);
	config_destroy(metadata);
	free(path_metadata);
}

pthread_mutex_t* esperar_acceso(char* nombre_pokemon){
	pthread_mutex_lock(&mutex_dict);
	pthread_mutex_t* mx_file = (pthread_mutex_t*) dictionary_get(sem_files, nombre_pokemon);
	pthread_mutex_unlock(&mutex_dict);

	while(1){
		pthread_mutex_lock(mx_file);
		if(!is_file_open(nombre_pokemon)){
			set_open_flag(nombre_pokemon, true);
			pthread_mutex_unlock(mx_file);
			return mx_file;
		} else {
			int tiempo_reintento = config_get_int_value(config, "TIEMPO_DE_REINTENTO_OPERACION");
			pthread_mutex_unlock(mx_file);
			sleep(tiempo_reintento);
		}
	}
}

void actualizar_metadata_y_ceder_acceso(char* nombre_pokemon, uint32_t file_size, t_list* bloques, pthread_mutex_t* mutex_file){
	pthread_mutex_lock(mutex_file);
	actualizar_metadata(nombre_pokemon, file_size, bloques, false);
	pthread_mutex_unlock(mutex_file);
}

void ceder_acceso(char* nombre_pokemon, pthread_mutex_t* mutex_file){
	pthread_mutex_lock(mutex_file);
	set_open_flag(nombre_pokemon, false);
	pthread_mutex_unlock(mutex_file);
}


// Se inicializa el semaforo del pokemon y (si asi se lo indica) se crea el archivo.
pthread_mutex_t* inicializar_pokemon(char* nombre_pokemon, bool new_file){
	char* lowered_nombre = string_duplicate(nombre_pokemon);
	string_to_lower(lowered_nombre);

	pthread_mutex_t* mx_pokemon = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(mx_pokemon, NULL);
	pthread_mutex_lock(mx_pokemon);

	pthread_mutex_lock(&mutex_dict);
	dictionary_put(sem_files, lowered_nombre, mx_pokemon);
	pthread_mutex_unlock(&mutex_dict);

	if(new_file){
		crear_file(lowered_nombre, true);
	}
	pthread_mutex_unlock(mx_pokemon);

	free(lowered_nombre);
	return mx_pokemon;
}


void new_pokemon(t_pokemon pokemon){
	t_list* coordenadas = NULL;
	t_list* bloques = NULL;
	int bytes_file;

	if(existe_pokemon(pokemon.nombre)){
		pthread_mutex_t* mutex_file = esperar_acceso(pokemon.nombre);
		get_pokemon_blocks_and_coordenadas(pokemon.nombre, &bloques, &coordenadas);
		add_coordenada(coordenadas, pokemon.posicion);
		bytes_file = escribir_en_filesystem(bloques, coordenadas);
		esperar_tiempo_retardo();
		actualizar_metadata_y_ceder_acceso(pokemon.nombre, bytes_file, bloques, mutex_file);
	} else {
		pthread_mutex_t* mutex_file = inicializar_pokemon(pokemon.nombre, true);
		bloques = list_create();
		coordenadas = list_create();
		add_coordenada(coordenadas, pokemon.posicion);

		bytes_file = escribir_en_filesystem(bloques, coordenadas);
		esperar_tiempo_retardo();
		actualizar_metadata_y_ceder_acceso(pokemon.nombre, bytes_file, bloques, mutex_file);
	}
	list_destroy_and_destroy_elements(coordenadas, free);
	list_destroy_and_destroy_elements(bloques, free);
}

uint32_t catch_pokemon(t_pokemon pokemon){
	t_list* coordenadas = NULL;
	t_list* bloques = NULL;
	int bytes_file;

	uint32_t catch_exitoso;
	if(existe_pokemon(pokemon.nombre)){
		pthread_mutex_t* mutex_file = esperar_acceso(pokemon.nombre);
		get_pokemon_blocks_and_coordenadas(pokemon.nombre, &bloques, &coordenadas);
		if(bloques == NULL) {
			esperar_tiempo_retardo();
			ceder_acceso(pokemon.nombre, mutex_file);
			catch_exitoso = 0;
		} else {
			catch_exitoso = restar_coordenada(coordenadas, pokemon.posicion);
			if(catch_exitoso) {
				bytes_file = escribir_en_filesystem(bloques, coordenadas);
				esperar_tiempo_retardo();
				actualizar_metadata_y_ceder_acceso(pokemon.nombre, bytes_file, bloques, mutex_file);
			} else {
				esperar_tiempo_retardo();
				ceder_acceso(pokemon.nombre, mutex_file);
			}
		}
	} else {
		catch_exitoso = 0;
	}
	return catch_exitoso;
}

uint32_t* get_pokemon(char* nombre_pokemon, uint32_t* cant_posiciones){
	t_list* coordenadas = NULL;
	t_list* bloques = NULL;

	uint32_t* posiciones;
	if(existe_pokemon(nombre_pokemon)){
		pthread_mutex_t* mutex_file = esperar_acceso(nombre_pokemon);
		get_pokemon_blocks_and_coordenadas(nombre_pokemon, &bloques, &coordenadas);
		if(bloques == NULL && coordenadas == NULL){
			posiciones = NULL;
			*cant_posiciones = 0;
		} else {
			posiciones = obtener_coordenadas(coordenadas, cant_posiciones);
		}
		esperar_tiempo_retardo();
		ceder_acceso(nombre_pokemon, mutex_file);
	} else {
		posiciones = NULL;
		*cant_posiciones = 0;
	}
	return posiciones;
}
