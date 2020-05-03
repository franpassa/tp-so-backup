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

#define PUERTO "6009"
#define PACKAGESIZE 1024

typedef struct{
	uint32_t id;
	uint32_t mensaje;
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

/*void recibir_mensajes(){
	int id_cola,id_tipo;

	while(1){

	id_tipo = // sacar info de mensaje
	id_cola = //sacar info de mensaje

	if (si tipo es original){
		char* mensaje = //sacar info de mensaje
			if (mensaje no esta en cola/){
				int id_mensaje = crear_id_mensaje();

				enviar_a_publisher_id(id_mensaje);

				t_info_mensaje nuevo_mensaje;
				nuevo_mensaje.id = id_mensaje;
				nuevo_mensaje.mensaje = mensaje;
				nuevo_mensaje.a_quienes_fue_enviado = NULL;
				nuevo_mensaje.cuantos_lo_recibieron = NULL;

				agregar_a_cola(id_cola,nuevo_mensaje);
				}
	} else {
		int id_correlativo = // sacar info de mensaje
		confirmar_mensaje(id_cola , id_correlativo);
	}
}*/

void enviar_a_publisher_id(int id){

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

//recibir mensaje() -- hilo del broker casi
//enviar_mensaje_a_suscriptores()
//estado_de_colas()

//MEMORIA
//dump_cache()
//compactar_memoria()



#endif /* BROKER_H_ */
