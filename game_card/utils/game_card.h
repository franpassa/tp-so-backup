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
#include <dirent.h>
#include <errno.h>

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

// -------- Globales --------

t_config* config;
t_log* logger;

// -------- Funciones --------

// --- General ---

t_config* get_config(char* config_path);
t_log* crear_log(char* log_path);
void terminar_aplicacion(char* mensaje);

// --- Filesystem ---

t_bitarray* inicializar_filesystem(char* punto_montaje);
void inicializar_bloques(char* punto_montaje, int cantidad);
t_bitarray* leer_bitmap(char* bitmap_path);
void print_bitarray(t_bitarray* bitarray);
t_bitarray* crear_bitmap(char* file_path, int cantidad_bloques);



#endif
