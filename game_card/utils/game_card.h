/*
 * game_card.h
 *
 *  Created on: 11 abr. 2020
 *      Author: utnso
 */

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

void init_fs(t_fspaths* paths);
void create_blocks(char* punto_montaje, int cantidad);
void print_bitarray(t_bitarray* bitarray);
t_bitarray* read_bitmap(char* bitmap_path);
t_bitarray* create_bitmap(char* file_path, int cantidad_bloques);
t_fspaths* init_fspaths(char* punto_montaje);
void free_fspaths(t_fspaths* paths);
void set_bit(char* bitmap_path, int index, bool value);

// --- Pokemons ---
int crear_metadata(char* path, bool is_file);
bool existe_pokemon(char* nombre_pokemon);
int crear_pokemon(char* nombre_pokemon);


#endif
