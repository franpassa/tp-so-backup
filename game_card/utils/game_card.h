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

// --- Filesystem ---

t_bitarray* inicializar_filesystem(char* punto_montaje);
void inicializar_bloques(char* punto_montaje, int cantidad);



#endif
