#include "game_card.h"

t_pokemon init_pokemon(char* nombre, uint32_t cantidad, uint32_t x, uint32_t y){
	t_pokemon pokemon;
	pokemon.nombre = nombre;
	pokemon.cantidad = cantidad;
	pokemon.x = x;
	pokemon.y = y;

	return pokemon;
}

bool existe_pokemon(char* nombre_pokemon){
	char* path_pokemon = string_new();
	char* lowered_pokemon = string_duplicate(nombre_pokemon);
	string_to_lower(lowered_pokemon);
	string_append_with_format(&path_pokemon, "%s/%s", fspaths->files_folder, lowered_pokemon);

	struct stat st = {0};
	bool pokemon_existe = stat(path_pokemon, &st) != -1;

	free(path_pokemon);
	free(lowered_pokemon);
	return pokemon_existe;
}

//Devuelve 0 si OK, -1 en caso de error
int crear_pokemon(t_pokemon pokemon){
	char* path_pokemon = string_new();
	char* lowered_pokemon = string_duplicate(pokemon.nombre);
	string_to_lower(lowered_pokemon);
	string_append_with_format(&path_pokemon, "%s/%s", fspaths->files_folder, lowered_pokemon);
	if(mkdir(path_pokemon , 0700) == -1){
		perror("Error creando pokemon");
		return -1;
	}
	crear_metadata(path_pokemon, true);
	//int free_block = get_free_block();

	free(path_pokemon);
	free(lowered_pokemon);
	return 0;
}

int crear_metadata(char* folder_path, bool is_file){
	char* metadata_path = string_duplicate(folder_path);
	string_append(&metadata_path, "/Metadata.bin");
	FILE* metadata = fopen(metadata_path, "w");
	if(metadata == NULL) {
		free(metadata_path);
		return -1;
	}
	if(is_file){
		fprintf(metadata, "DIRECTORY=N\n"
						  "SIZE=0\n"
						  "BLOCKS=[]\n"
						  "OPEN=N");
	} else {
		fprintf(metadata, "DIRECTORY=Y");
	}
	free(metadata_path);
	fclose(metadata);
	return 0;
}
