#ifndef GAME_CARD_H_
#define GAME_CARD_H_


// -------- Librerias C estandar --------

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

// -------- Commons --------

#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/bitarray.h>

// -------- Cuarenteam libs --------

#include <conexiones.h>

// -------- Constantes --------

#define CONFIG_PATH "game_card.config"
#define LOG_PATH "game_card.log"

// -------- Structs --------

typedef struct {
	char* bitmap_file;
	char* metadata_file;
	char* blocks_folder;
	char* files_folder;
} t_fspaths;

typedef struct {
	char* nombre;
	uint32_t cantidad;
	uint32_t x;
	uint32_t y;
} t_pokemon;

// -------- Globales --------

t_config* config;
t_log* logger;
t_fspaths* fspaths;

// -------- Funciones --------

// --- General ---

t_config* get_config(char* config_path);
t_log* crear_log(char* log_path);
void terminar_aplicacion(char* mensaje);
long get_file_size(FILE* file_ptr);

// --- Filesystem ---

t_fspaths* init_fspaths(char* punto_montaje);
void init_fs();
int create_bitmap(int cantidad_bloques);
int create_blocks(int cantidad);
t_bitarray* read_bitmap(long* size_in_bytes);
void print_bitarray(t_bitarray* bitarray);
void set_bit(int index, bool value);
int get_free_block();
void free_fspaths(t_fspaths* paths);

// --- Pokemons ---
t_pokemon init_pokemon(char* nombre, uint32_t cantidad, uint32_t x, uint32_t y);
int crear_metadata(char* path, bool is_file);
bool existe_pokemon(char* nombre_pokemon);
int crear_pokemon(t_pokemon pokemon);


#endif
