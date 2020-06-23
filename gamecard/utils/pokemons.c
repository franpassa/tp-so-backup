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

//0 si OK, -1 error
int crear_pokemon(t_pokemon pokemon){
	char* path_pokemon = get_pokemon_path(pokemon.nombre);

	if(mkdir(path_pokemon , 0700) == -1){
		perror("Error creando pokemon");
	} else {
		uint32_t bytes_escritos;
<<<<<<< HEAD:gamecard/utils/pokemons.c
		t_list* bloques = escribir_en_bloques(pokemon, -1, &bytes_escritos); // Se pasa ultimo_bloque como -1 para que se escriba uno nuevo.
=======
		t_list* bloques = escribir_en_filesystem(pokemon, NULL, &bytes_escritos); // Se pasa ultimo_bloque como -1 para que se escriba uno nuevo.
>>>>>>> 7a3b519ce534f7b5e92b4860df42e87b899b2ff0:gamecard/utils/pokemons.c
		if(bloques != NULL){
			crear_metadata(path_pokemon, bytes_escritos, bloques);
		} else {
			printf("Error en 'crear_pokemon'");
		}
	}

	free(path_pokemon);
	return 0;
}

<<<<<<< HEAD:gamecard/utils/pokemons.c
char* get_last(char** array){
	int index = 0;
	while(array[index] != NULL) index++;

	char* elemento = array[index-1];
	return elemento;
}

int obtener_ultimo_bloque(char* nombre_pokemon){
	char* path_metadata_pok = get_metadata_path(nombre_pokemon);

	t_config* pokemon_metadata = config_create(path_metadata_pok);
	char** bloques = config_get_array_value(pokemon_metadata, "BLOCKS");

	char* elemento = get_last(bloques);
	int last_block;
	sscanf(elemento, "%d", &last_block);
	config_destroy(pokemon_metadata);

	return last_block;
=======
void toggle_open_flag(char* nombre_pokemon){
	char* path_metadata = get_pokemon_path(nombre_pokemon);
	string_append(&path_metadata, "/Metadata.bin");
	t_config* metadata_config = config_create(path_metadata);
	if(metadata_config != NULL){
		char* open_status = config_get_string_value(metadata_config, "OPEN");

		if(open_status[0] == 'N'){
			config_set_value(metadata_config, "OPEN", "Y");
		} else {
			config_set_value(metadata_config, "OPEN", "N");
		}
		config_save(metadata_config);
		config_destroy(metadata_config);
	}
	free(path_metadata);
}

bool is_file_open(char* nombre_pokemon){
	char* path_metadata = get_pokemon_path(nombre_pokemon);
	string_append(&path_metadata, "/Metadata.bin");
	t_config* metadata_config = config_create(path_metadata);
	if(metadata_config != NULL){
		char* open_status = config_get_string_value(metadata_config, "OPEN");

		return open_status[0] == 'Y';
	} else {
		return false;
	}
>>>>>>> 7a3b519ce534f7b5e92b4860df42e87b899b2ff0:gamecard/utils/pokemons.c
}
