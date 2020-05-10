#ifndef SERVER_H_
#define SERVER_H_


#include "broker.h"

void iniciar_servidor(){

		struct addrinfo hints;
		struct addrinfo *serverInfo;

		//char* ip = config_get_string_value(config,"IP_BROKER");
		//char* puerto = config_get_string_value(config,"PUERTO_BROKER");

		char* ip = "127.0.0.1";
		char* puerto = "6012";

		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags = AI_PASSIVE;

		getaddrinfo(ip, puerto, &hints, &serverInfo);

		int listeningSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);

		if(listeningSocket == -1){
			printf("error\n");
			exit(-1);
		}

		if(bind(listeningSocket,serverInfo->ai_addr, serverInfo->ai_addrlen) == -1){
			printf("error en bind\n");
			exit(-1);
		}
		//if para ver si se bindeo bien

		listen(listeningSocket, SOMAXCONN);

		freeaddrinfo(serverInfo);

		printf("Estoy escuchando\n");

		while(1){
			esperar_cliente(listeningSocket);
		}


}


void esperar_cliente(int socket_servidor){ // Hilo esperar_cliente

	while(1){

		struct sockaddr_in dir_cliente;

		int tam_direccion = sizeof(struct sockaddr_in);

		int socket_cliente = accept(socket_servidor, (void*) &dir_cliente, (socklen_t*) &tam_direccion);

		queue_name cola;

		if(recv(socket_cliente, &cola, sizeof(queue_name), MSG_WAITALL) == -1) {
			perror("error msg:");
			printf("error recibiendo mensaje\n"); //log de error
			continue; // vuelve al loop

		}


		if(suscribir_a_cola(socket_cliente, cola) == -1){
			printf("%d es un codigo invalido\n", cola);
		} else {
			printf("el cliente %d se suscribio a la cola %s\n", socket_cliente, nombres_colas[cola]);
		}

	}

}

int suscribir_a_cola(int socket_cliente, queue_name cola) {	// *socket_cliente porque el hilo recibe punteros

	// Agregar Semaforo


	//pthread_mutex_lock(&semaforo_suscriber);

	switch(cola){

	case NEW_POKEMON:

		list_add(QUEUE_NEW_POKEMON.lista_suscriptores,&socket_cliente);

		break;

	case APPEARED_POKEMON:

		list_add(QUEUE_APPEARED_POKEMON.lista_suscriptores,&socket_cliente);

		break;

	case CATCH_POKEMON:

		list_add(QUEUE_CATCH_POKEMON.lista_suscriptores,&socket_cliente);

		break;

	case CAUGHT_POKEMON:

		list_add(QUEUE_CAUGHT_POKEMON.lista_suscriptores,&socket_cliente);

		break;

	case GET_POKEMON:

		list_add(QUEUE_GET_POKEMON.lista_suscriptores,&socket_cliente);

		break;

	case LOCALIZED_POKEMON:

		list_add(QUEUE_LOCALIZED_POKEMON.lista_suscriptores,&socket_cliente);

		break;

	default:
		// manejar error: codigo bien pero es invalido
		return -1;

	}

	return 0;
	//pthread_mutex_unlock(&semaforo_suscriber);

}




#endif /* SERVER_H_ */
