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

	t_config* metadata_config = config_create(path_metadata);
	char* open_status = config_get_string_value(metadata_config, "OPEN");
	bool is_open = open_status[0] == 'Y';

	free(path_metadata);
	config_destroy(metadata_config);

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

void new_pokemon(t_pokemon pokemon){
	t_list* coordenadas = NULL;
	t_list* bloques = NULL;
	int bytes_file;

	if(existe_pokemon(pokemon.nombre)){
		pthread_mutex_t* mx_file = (pthread_mutex_t*) dictionary_get(sem_files, pokemon.nombre);
		printf("%d\n", mx_file != NULL);
		while(is_file_open(pokemon.nombre)){
			printf("Archivo %s está abierto, esperando para acceder...", pokemon.nombre);
			sleep(10);
		}
		pthread_mutex_lock(mx_file);
		set_open_flag(pokemon.nombre, true);
		pthread_mutex_unlock(mx_file);
		get_pokemon_blocks_and_coordenadas(pokemon.nombre, &bloques, &coordenadas);
		add_coordenada(coordenadas, pokemon.posicion);

		bytes_file = escribir_en_filesystem(pokemon, bloques, coordenadas);

		actualizar_metadata(pokemon.nombre, bytes_file, bloques);
		set_open_flag(pokemon.nombre, false);
	} else {
		pthread_mutex_t* mx_file = malloc(sizeof(pthread_mutex_t));
		pthread_mutex_init(mx_file, NULL);
		dictionary_put(sem_files, pokemon.nombre, mx_file);
		crear_file(pokemon.nombre, true);
		pthread_mutex_lock(mx_file);
		pthread_mutex_unlock(mx_file);
		bloques = list_create();
		coordenadas = list_create();
		add_coordenada(coordenadas, pokemon.posicion);

		bytes_file = escribir_en_filesystem(pokemon, bloques, coordenadas);
		actualizar_metadata(pokemon.nombre, bytes_file, bloques);
		set_open_flag(pokemon.nombre, false);
	}
}
