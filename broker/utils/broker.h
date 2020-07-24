#ifndef BROKER_H_
#define BROKER_H_

// COMMONS
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/txt.h>

// ESTANDAR C
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <math.h>
// CONEXIONES
#include <sys/socket.h>
#include <conexiones.h>
#include <arpa/inet.h>

typedef struct {
	uint32_t id;
} t_suscriptor;

typedef struct {
	t_list* lista_suscriptores;
	queue_name tipo_cola;
} t_cola_de_mensajes;

// MEMORIA

typedef struct {
	uint32_t tipo_mensaje;
	uint32_t tamanio;
	uint32_t id_mensaje;
	uint32_t bit_inicio;
	uint32_t auxiliar;
	uint32_t id_correlativo;
	t_list* a_quienes_fue_enviado;
	t_list* quienes_lo_recibieron;
}t_struct_secundaria;

// Variables globales memoria

void* memoria;

int flag;
int entra;
int tamanio_a_ocupar;
char* algoritmo_memoria;
char* algoritmo_remplazo;
char* algoritmo_part_libre;
int frecuencia_compactacion;
int tamanio_minimo;
int cont_orden;
int cont_lru;

uint32_t tamanio_memoria;

t_list* lista_de_particiones;

// VARIABLES GLOBALES

t_cola_de_mensajes* queue_new_pokemon;
t_cola_de_mensajes* queue_appeared_pokemon;
t_cola_de_mensajes* queue_catch_pokemon;
t_cola_de_mensajes* queue_caught_pokemon;
t_cola_de_mensajes* queue_get_pokemon;
t_cola_de_mensajes* queue_localized_pokemon;

t_list* sockets_productores;

t_log* logger;
t_config* config;

int contador_id;
char* nombres_colas[7];


// SEMAFOROS

sem_t semaforo_contador_productores;
pthread_mutex_t semaforo_id;
pthread_mutex_t sem_cola[6];
pthread_mutex_t mutex_productores;
pthread_mutex_t semaforo_struct_s;
pthread_mutex_t semaforo_memoria;
pthread_mutex_t sem_lru;
sem_t binario_mandar;

// HILOS

pthread_t hilo_estado_queues;
pthread_t hilo_suscripciones;
pthread_t hilo_mensajes;
pthread_t hilo_enviar_mensaje;


// FUNCIONES DE QUEUES

void inicializar_cola(t_cola_de_mensajes**, queue_name);
void inicializar_colas();
t_cola_de_mensajes* int_a_nombre_cola(queue_name);
void inicializar();
void mostrar_estado_de_una_queue(t_cola_de_mensajes*);
void estado_de_queues();
void print_list_sockets(void* numero);
void print_mensaje_de_cola(t_struct_secundaria*);
void print_list_sockets_de_un_mensaje(void*);
void print_list_sockets_ACK_de_un_mensaje(void*);
void recorrer_cola_de_mensajes_para_mostrar(t_cola_de_mensajes*);
void free_queue(t_cola_de_mensajes*);
void free_particion(void*);
// FUNCIONES DE LOG Y CONFIG

t_log* iniciar_logger(void);
t_config* leer_config(void);
void terminar_programa(t_log*, t_config*);


// Server

int iniciar_servidor_broker();
void esperar_cliente(int*);
int suscribir_a_cola(int, queue_name);

// Recibir

void loop_productores();
void recibir_mensajes_para_broker(uint32_t*);
void confirmar_mensaje(queue_name, uint32_t,uint32_t);
uint32_t crear_nuevo_id();
void agregar_a_cola(uint32_t,uint32_t,uint32_t);
bool es_el_mismo_mensaje(queue_name, void*,void*);
uint32_t revisar_si_mensaje_no_estaba_en_cola(queue_name, void*, uint32_t);

// Mandar

void mandar_mensajes();
bool esta_en_lista(t_list*, uint32_t*);
int mandar(queue_name, void*, int,int,int,uint32_t);
void recorrer_struct_s();

// MEMORIA

void inicializar_memoria();
void almacenar(void*, uint32_t, uint32_t, uint32_t,uint32_t);
t_struct_secundaria* duplicar_estructura(t_struct_secundaria*);
void llenar_estructura(t_struct_secundaria*, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
void buscar_particion_en_particiones_dinamicas();
void compactar();
void consolidar();
void elegir_victima_para_eliminar_mediante_FIFO_o_LRU_particiones();
void elegir_victima_para_eliminar_mediante_FIFO_o_LRU_bs();
void liberar_memoria_interna();
void actualizar_bit_inicio(int);
void mover_memoria(int);
void dump_de_cache(int sig);

void buscar_particion_en_bs();
void consolidar_particiones_en_bs(int posicion_liberada);
bool son_buddies(t_struct_secundaria*, t_struct_secundaria*);
bool es_potencia_de_dos(int);
int elegir_bit_aux_mas_viejo();
t_struct_secundaria* encontrar_particion_en_base_a_un_id_mensaje(uint32_t,int);
void capturar_senial();
int mayor_entre_Min_y_tam(int);
int f_cont_lru();
void* sacar_mensaje_de_memoria(uint32_t,uint32_t);
int potencia (int base,int expo);
#endif /* BROKER_H_ */
