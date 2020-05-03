#ifndef BROKER_H_
#define BROKER_H_

#include <stdio.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include <commons/log.h>
#include <stdint.h>
#include <pthread.h>
#include <commons/config.h>
#include <sys/socket.h>
#include <conexiones.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <signal.h>

#define PUERTO "6009"
#define PACKAGESIZE 1024

int contador_id = 0;
int semaforo_id = 1;

typedef struct{
	uint32_t id;
	t_paquete paquete;
	uint32_t a_quienes_fue_enviado;
	uint32_t cuantos_lo_recibieron;
} t_info_mensaje;

typedef struct{
	uint32_t id;
} t_suscriptor;

typedef struct{
	t_queue* cola;
	t_list* lista_suscriptores;
}t_cola_de_mensajes;

t_cola_de_mensajes QUEUE_NEW_POKEMON;
t_cola_de_mensajes QUEUE_APPEARED_POKEMON;
t_cola_de_mensajes QUEUE_CATCH_POKEMON;
t_cola_de_mensajes QUEUE_CAUGHT_POKEMON;
t_cola_de_mensajes QUEUE_GET_POKEMON;
t_cola_de_mensajes QUEUE_LOCALIZED_POKEMON;

t_log* logger;
t_config* config;

pthread_t thread;

t_log* iniciar_logger(void);
t_config* leer_config(void);
void terminar_programa(t_log*, t_config*);
int crear_nuevo_id();


t_cola_de_mensajes inicializar_cola(t_cola_de_mensajes nombre_cola){

	t_queue* nueva_cola = queue_create();
	nombre_cola.cola = nueva_cola;
	nombre_cola.lista_suscriptores = list_create();
	return nombre_cola;
}

bool revisar_mensaje(int id_cola , t_buffer mensaje);

void  agregar_a_cola(int id_cola, t_buffer mensaje);

void confirmar_mensaje(int id_cola ,int  id_mensaje);

void enviar_a_publisher_id(int id){

}
t_cola_de_mensajes int_a_nombre_cola(int id){
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
		}
}
//servidor
void iniciar_servidor(void){

		struct addrinfo hints;
		struct addrinfo *serverInfo;

		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_flags = AI_PASSIVE;
		hints.ai_socktype = SOCK_STREAM;

		getaddrinfo(NULL, PUERTO, &hints, &serverInfo);

		int listenningSocket;
		listenningSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);

		bind(listenningSocket,serverInfo->ai_addr, serverInfo->ai_addrlen);
		freeaddrinfo(serverInfo);

		listen(listenningSocket, SOMAXCONN);

		printf("Estoy escuchando\n");



		close(listenningSocket);

}




#endif /* BROKER_H_ */
