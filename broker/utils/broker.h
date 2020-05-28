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
	queue_name tipo_cola;
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



// FUNCIONES DE QUEUES

void inicializar_cola(t_cola_de_mensajes** nombre_cola, queue_name cola);
void inicializar_colas();
t_cola_de_mensajes* int_a_nombre_cola(queue_name id);
void inicializar();
void mostrar_estado_de_una_queue(t_cola_de_mensajes* cola);
void estado_de_queues();
void print_list_sockets(void* numero);
void print_mensaje_de_cola(t_info_mensaje*);
void print_list_sockets_de_un_mensaje(void* numero);
void recorrer_cola_de_mensajes_para_mostrar(t_cola_de_mensajes*);

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
void recibir_mensajes_para_broker(int* socket_cliente);
void confirmar_mensaje(queue_name,uint32_t);
uint32_t crear_nuevo_id();
void agregar_a_cola(uint32_t,t_paquete*, int);
bool es_el_mismo_mensaje(queue_name, void*,void*);
bool revisar_si_mensaje_no_estaba_en_cola(queue_name, void*);

// Mandar

void mandar_mensajes();
bool esta_en_lista(t_list*,uint32_t*);
void mandar(t_paquete* paquete,uint32_t id, uint32_t sub);
void recorrer_cola(t_cola_de_mensajes*);


#endif /* BROKER_H_ */
