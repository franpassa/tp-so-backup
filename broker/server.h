#ifndef SERVER_H_
#define SERVER_H_


#include "broker.h"

void iniciar_servidor(char* ip,char* puerto){

		struct addrinfo hints;
		struct addrinfo *serverInfo;

		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_flags = AI_PASSIVE;
		hints.ai_socktype = SOCK_STREAM;

		getaddrinfo(ip, puerto, &hints, &serverInfo);

		int listeningSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);

		bind(listeningSocket,serverInfo->ai_addr, serverInfo->ai_addrlen);
		freeaddrinfo(serverInfo);

		listen(listeningSocket, SOMAXCONN);

		printf("Estoy escuchando\n");


}


void esperar_cliente(int* socket_servidor){ // Hilo esperar_cliente

	while(1){
		struct sockaddr_in dir_cliente;
		int tam_direccion = sizeof(struct sockaddr_in);

		int socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);
		int cola;
		if(recv(*socket_servidor, &cola, MSG_WAITALL) == -1) cola = -1;

		suscribir_a_cola(socket_cliente, cola);

		printf("El socket %d se suscribio a la cola %d\n", socket_cliente, cola);

	}

}

void suscribir_a_cola(int *socket_cliente, int cola) {	// *socket_cliente porque el hilo recibe punteros

	// Agregar Semaforo

	switch(cola){

	case 0:

		list_add(QUEUE_NEW_POKEMON.lista_suscriptores,socket_cliente);

		break;

	case 1:

		list_add(QUEUE_APPEARED_POKEMON.lista_suscriptores,socket_cliente);

		break;

	case 2:

		list_add(QUEUE_CATCH_POKEMON.lista_suscriptores,socket_cliente);

		break;

	case 3:

		list_add(QUEUE_CAUGHT_POKEMON.lista_suscriptores,socket_cliente);

		break;

	case 4:

		list_add(QUEUE_GET_POKEMON.lista_suscriptores,socket_cliente);

		break;

	case 5:

		list_add(QUEUE_LOCALIZED_POKEMON.lista_suscriptores,socket_cliente);

		break;

	default:

		break;

	}


}

void process_request(int *socket_cliente){

}



#endif /* SERVER_H_ */
