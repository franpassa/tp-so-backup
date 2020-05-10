#ifndef BROKER_H_
#define BROKER_H_

#include <stdio.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <commons/log.h>
#include <stdint.h>
#include <pthread.h>
#include <commons/config.h>
#include <sys/socket.h>
#include <conexiones.h>
#include <arpa/inet.h>
#include <semaphore.h>

#define PUERTO "6009"


typedef struct{
	uint32_t id;
	t_paquete* paquete;
	t_list* a_quienes_fue_enviado;
	uint32_t cuantos_lo_recibieron;
} t_info_mensaje;

typedef struct{
	uint32_t id;
} t_suscriptor;

typedef struct{
	t_queue* cola;
	t_list* lista_suscriptores;
}t_cola_de_mensajes;

pthread_t thread;

int contador_id = 0;

pthread_mutex_t semaforo_id;
pthread_mutex_t semaforo_suscriber;
pthread_mutex_t sem_cola[6] = (1,1,1,1,1,1);

//Cola De Mensajes

t_cola_de_mensajes QUEUE_NEW_POKEMON;
t_cola_de_mensajes QUEUE_APPEARED_POKEMON;
t_cola_de_mensajes QUEUE_CATCH_POKEMON;
t_cola_de_mensajes QUEUE_CAUGHT_POKEMON;
t_cola_de_mensajes QUEUE_GET_POKEMON;
t_cola_de_mensajes QUEUE_LOCALIZED_POKEMON;

t_cola_de_mensajes inicializar_cola(t_cola_de_mensajes nombre_cola);
t_cola_de_mensajes int_a_nombre_cola(uint32_t id);

//Loggers y Config

t_log* logger;
t_config* config;

char* nombres_colas[] = {"NEW_POKEMON","APPEARED_POKEMON","CATCH_POKEMON","CAUGHT_POKEMON","GET_POKEMON","LOCALIZED_POKEMON"};

t_log* iniciar_logger(void);

t_config* leer_config(void);

void terminar_programa(t_log*, t_config*);


// recibir.h

void recibir_mensajes();

bool no_esten_en(t_list* a_los_que_envie,uint32_t sub);

bool es_el_mismo_mensaje(uint32_t id, void* mensaje,void* otro_mensaje);

void  agregar_a_cola(uint32_t id_cola, t_paquete* mensaje);

void confirmar_mensaje(uint32_t id_cola ,uint32_t  id_mensaje);

void enviar_a_publisher_id(uint32_t id){
	// hacer
};

int crear_nuevo_id();

//mandar.h

void recorrer_cola(t_cola_de_mensajes nombre);

void mandar_mensajes();

void mandar(t_paquete* paquete, uint32_t sub);

void enviar_a(t_paquete* paquete,t_list* sin_enviar);

bool revisar_mensaje(uint32_t id_cola , t_buffer* mensaje);

bool igual_a(void* uno ,uint32_t otro);

//server.h

void iniciar_servidor();
void esperar_cliente(int socket_servidor);
int suscribir_a_cola(int socket_cliente, queue_name cola);

t_cola_de_mensajes inicializar_cola(t_cola_de_mensajes nombre_cola){

	t_queue* nueva_cola = queue_create();

	nombre_cola.cola = nueva_cola;

	nombre_cola.lista_suscriptores = list_create();

	return nombre_cola;
}

t_cola_de_mensajes int_a_nombre_cola(uint32_t id){
  //cambiar del 0 - 5
	switch (id){
		case 1:
			return QUEUE_NEW_POKEMON;
			break;
		case 2:
			return QUEUE_APPEARED_POKEMON;
			break;
		case 3:
			return QUEUE_CATCH_POKEMON;
			break;
		case 4:
			return QUEUE_CAUGHT_POKEMON;
			break;
		case 5:
			return QUEUE_GET_POKEMON;
			break;
		case 6:
			return QUEUE_LOCALIZED_POKEMON;
			break;

		default: ;
			t_cola_de_mensajes queue_null;
			return queue_null;
		}
}





#endif /* BROKER_H_ */
