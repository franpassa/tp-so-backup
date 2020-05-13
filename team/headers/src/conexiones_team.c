#include "../conexiones_team.h"


void enviar_gets(t_list* objetivos_globales){

	void enviar(void* especie)
	{
		/*IMPORTO EL IP Y EL PUERTO DEL BROKER */
		char* ip_broker = config_get_string_value(config,"IP_BROKER");
		char* puerto_broker = config_get_string_value(config, "PUERTO_BROKER");
		/*CREO CONEXION*/
		int socket_para_envio = conectar_como_productor(ip_broker,puerto_broker); // ESTA FUNCION ESTA EN DUDA, PORQUE LA COLA PRODUCTOR NO CREO QUE SE MANTENGA
		if(socket_para_envio == -1){
			printf("Fallo la conexion con el Broker.\n");
		}
		/*CREO EL MENSAJE Y LO ENVIO*/
		char* mensaje = ((t_especie*)especie)->especie;
		get_pokemon_msg* a_enviar = get_msg(mensaje);
		int* id_respuesta = malloc(sizeof(int));
		*id_respuesta = enviar_mensaje(GET_POKEMON,a_enviar,socket_para_envio);
		/*AGREGO EL ID A LA LISTA DE IDS ENVIADOS*/
		list_add(ids_get,id_respuesta);
		/*CIERRO CONEXION*/
		close(socket_para_envio);
	}

	list_iterate(objetivos_globales,enviar);
}

int suscribirse_a_queue(queue_name cola,char* ip_broker, char* puerto_broker){

	int socket_cola = 0;

	if(cola == APPEARED_POKEMON){
		socket_cola = suscribirse_a_cola(APPEARED_POKEMON,ip_broker,puerto_broker);
		if(socket_cola == -1){
			printf("Error al suscribirse a la cola 'APPEARED POKEMON'.\n\n");
			return socket_cola;
		}
	}else if(cola == CAUGHT_POKEMON){
		socket_cola = suscribirse_a_cola(CAUGHT_POKEMON,ip_broker,puerto_broker);
		if(socket_cola == -1){
			printf("Error al suscribirse a la cola 'CAUGHT POKEMON'.\n\n");
			return socket_cola;
		}
	}else if(cola == LOCALIZED_POKEMON){
		socket_cola = suscribirse_a_cola(LOCALIZED_POKEMON,ip_broker,puerto_broker);
		if(socket_cola == -1){
			printf("Error al suscribirse a la cola 'LOCALIZED POKEMON'.\n\n");
			return socket_cola;
		}
	}
	return socket_cola;
}

void recibirLocalized(int* socket_localized){ // no esta terminada
	char* ip_broker = config_get_string_value(config,"IP_BROKER");
	char* puerto_broker = config_get_string_value(config, "PUERTO_BROKER");
	int tiempo_reconexion = config_get_int_value(config, "TIEMPO_RECONEXION");
	socket_localized = suscribirse_a_cola(LOCALIZED_POKEMON,ip_broker,puerto_broker);

	while(socket_localized == -1){
		socket_localized = suscribirse_a_cola(LOCALIZED_POKEMON,ip_broker,puerto_broker);
		sleep(tiempo_reconexion);
	}

	while(1){
		if(recibir_mensaje(LOCALIZED_POKEMON,*socket_localized) != NULL){
			mensaje_recibido_localized = recibir_mensaje(LOCALIZED_POKEMON,*socket_localized);
		}

	}
}

void esperar_cliente(int* socket_servidor){
	while(1){

		struct sockaddr_in dir_cliente;

		int tam_direccion = sizeof(struct sockaddr_in);

		int socket_cliente = accept(*socket_servidor, (void*) &dir_cliente, (socklen_t*) &tam_direccion);
		printf("Nuevo cliente entrante: %d\n", socket_cliente);

		queue_name cola;

		if(recv(socket_cliente, &cola, sizeof(queue_name), MSG_WAITALL) == -1) {
			perror("Error al recibir el mensaje:");
			printf("Error recibiendo mensaje\n");
			continue; // vuelve al loop
		}

		printf("El cliente %d se suscribió correctamente", socket_cliente);
	}
}

