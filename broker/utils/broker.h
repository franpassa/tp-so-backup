#ifndef BROKER_H_
#define BROKER_H_

// COMMONS
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <commons/log.h>
#include <commons/config.h>

// ESTANDAR C
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>

// CONEXIONES
#include <sys/socket.h>
#include <conexiones.h>
#include <arpa/inet.h>

typedef struct {
	uint32_t id;
	t_paquete* paquete;
	t_list* a_quienes_fue_enviado;
	uint32_t cuantos_lo_recibieron;
} t_info_mensaje;

typedef struct {
	uint32_t id;
} t_suscriptor;

typedef struct {
	t_queue* cola;
	t_list* lista_suscriptores;
} t_cola_de_mensajes;

// VARIABLES GLOBALES

t_cola_de_mensajes* QUEUE_NEW_POKEMON;
t_cola_de_mensajes* QUEUE_APPEARED_POKEMON;
t_cola_de_mensajes* QUEUE_CATCH_POKEMON;
t_cola_de_mensajes* QUEUE_CAUGHT_POKEMON;
t_cola_de_mensajes* QUEUE_GET_POKEMON;
t_cola_de_mensajes* QUEUE_LOCALIZED_POKEMON;

t_list* sockets_productores;

t_log* logger;
t_config* config;

int contador_id;
char* nombres_colas[7];

// SEMAFOROS

pthread_mutex_t semaforo_id;
pthread_mutex_t semaforo_suscriber;
pthread_mutex_t sem_cola[6];
pthread_mutex_t mutex_productores;
int cont_cola[6]; // pthread_mutex_trylock

// FUNCIONES DE QUEUES

void inicializar_cola(t_cola_de_mensajes** nombre_cola);
void inicializar_colas();
t_cola_de_mensajes* int_a_nombre_cola(queue_name id);
void inicializar();
void mostrar_subs(t_cola_de_mensajes* cola);
void estado_de_queues();
// FUNCIONES DE LOG Y CONFIG

t_log* iniciar_logger(void);
t_config* leer_config(void);
void terminar_programa(t_log*, t_config*);

// Server

int iniciar_servidor_broker();
void esperar_cliente(int* socket_cliente);
int suscribir_a_cola(int,queue_name);

// Recibir

void loop_productores();
void chequear_mensajes(int* socket_cliente);
void confirmar_mensaje(queue_name,uint32_t);
int crear_nuevo_id();
void agregar_a_cola(queue_name,t_paquete*);
bool es_el_mismo_mensaje(queue_name, void*,void*);
bool revisar_mensaje(queue_name, t_buffer*);

// Mandar

void mandar_mensajes();
bool igual_a(void*,uint32_t);
bool no_esten_en(t_list*,uint32_t);
void mandar(t_paquete*,uint32_t);
void enviar_a(t_paquete*,t_list*);
void recorrer_cola(t_cola_de_mensajes*);

// ver estado de colas con sus mensajes y sucriptores

#endif /* BROKER_H_ */
