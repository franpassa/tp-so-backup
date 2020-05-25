#include "../conexiones_team.h"

void enviar_gets(t_list* objetivos_globales) {

	void enviar(void* especie) {
		/*CREO EL MENSAJE Y LO ENVIO*/
		char* mensaje = ((t_especie*) especie)->especie;
		get_pokemon_msg* a_enviar = get_msg(mensaje);
		uint32_t* id_respuesta = malloc(sizeof(uint32_t));
		*id_respuesta = enviar_mensaje(ip_broker, puerto_broker,GET_POKEMON,a_enviar,true);
		if (*id_respuesta == -1) {
			printf("Fallo la conexion con el Broker.\n");
		} else {
			/*AGREGO EL ID A LA LISTA DE IDS ENVIADOS*/
			list_add(ids_enviados, id_respuesta);
		}
	}

	list_iterate(objetivos_globales, enviar);
}

int suscribirse_a_queue(queue_name cola, char* ip_broker, char* puerto_broker) {

	int socket_cola = 0;

	if (cola == APPEARED_POKEMON) {
		socket_cola = suscribirse_a_cola(APPEARED_POKEMON, ip_broker,puerto_broker);
		if (socket_cola == -1) {
			printf("Error al suscribirse a la cola 'APPEARED POKEMON'.\n\n");
			return socket_cola;
		}
	} else if (cola == CAUGHT_POKEMON) {
		socket_cola = suscribirse_a_cola(CAUGHT_POKEMON, ip_broker,puerto_broker);
		if (socket_cola == -1) {
			printf("Error al suscribirse a la cola 'CAUGHT POKEMON'.\n\n");
			return socket_cola;
		}
	} else if (cola == LOCALIZED_POKEMON) {
		socket_cola = suscribirse_a_cola(LOCALIZED_POKEMON, ip_broker,puerto_broker);
		if (socket_cola == -1) {
			printf("Error al suscribirse a la cola 'LOCALIZED POKEMON'.\n\n");
			return socket_cola;
		}
	}
	return socket_cola;
}

void recibirLocalized() { // no esta terminada

	char* ip_broker = config_get_string_value(config, "IP_BROKER");
	char* puerto_broker = config_get_string_value(config, "PUERTO_BROKER");
	int tiempo_reconexion = config_get_int_value(config, "TIEMPO_RECONEXION");
	socket_localized = suscribirse_a_cola(LOCALIZED_POKEMON, ip_broker,puerto_broker);

	while (socket_localized == -1) {
		int intento = 1;
		socket_localized = suscribirse_a_cola(LOCALIZED_POKEMON, ip_broker,	puerto_broker);
		printf("Intento de reconexion numero %d.", intento);
		sleep(tiempo_reconexion);
		intento++;
	}

	printf("Conexion con la cola de mensajes LOCALIZED_POKEMON exitosa. Esperando mensajes...");

	while (1) {
		mensaje_recibido_localized = recibir_mensaje(LOCALIZED_POKEMON,socket_localized);

		if (mensaje_recibido_localized != NULL) {
			if ((estaEnLaLista((mensaje_recibido_localized->nombre_pokemon),objetivos_globales)) && (!(estaEnLaLista((mensaje_recibido_localized->nombre_pokemon),pokemons_recibidos)))) {
				agregarPokemonsRecibidosALista(pokemons_recibidos,mensaje_recibido_localized);
				t_list* todosLosEntrenadores = todosLosEntrenadoresAPlanificar();
				t_entrenador* entrenadorReady = entrenadorAReady(todosLosEntrenadores, pokemons_recibidos);

				bool esElMismo(void* entrenador) {
					return entrenador == entrenadorReady;
				}

				t_entrenador* removido = list_remove_by_condition(listaALaQuePertenece(entrenadorReady), esElMismo);
				list_add(estado_ready, removido);
			}
		}
	}

	close(socket_localized);
}

void esperar_cliente(int* socket_servidor) {
// ES UN ASCO

	while (1) {

		struct sockaddr_in dir_cliente;

		int tam_direccion = sizeof(struct sockaddr_in);

		int socket_cliente = accept(*socket_servidor, (void*) &dir_cliente,(socklen_t*) &tam_direccion);
		printf("Nuevo cliente entrante: %d\n", socket_cliente);

		queue_name cola;

		if (recv(socket_cliente, &cola, sizeof(queue_name), MSG_WAITALL) == -1) {
			perror("Error al recibir el mensaje:");
			continue; // vuelve al loop
		}

		if (cola != PRODUCTOR) {
			printf("flasheaste papi, cola incorrecta\n");
			continue;
		}
		// ESTE TP DESTRUIRÁ MI CORDURA
		queue_name otra_cola;
		int bytes = recv(socket_cliente, &otra_cola, sizeof(queue_name),MSG_WAITALL);
		if (bytes != -1) {
			appeared_pokemon_msg* msg = recibir_mensaje(APPEARED_POKEMON,socket_cliente);
			printf("APPEARED POKEMON RECIBIDO: nombre: %s, X: %d, Y: %d\n",msg->nombre_pokemon, msg->coordenada_X, msg->coordenada_Y);
			free(msg->nombre_pokemon);
			free(msg);
		} else {
			perror("ups");
		}
	}
}

void estado_exec()
{
	while (1)
	{
		if (!list_is_empty(estado_ready))
		{
			t_entrenador* unEntrenador = (t_entrenador*) list_get(estado_ready,0);
			bool esElMismo(void* entrenador)
			{
				return entrenador == unEntrenador;
			}
			hayEntrenadorProcesando = true;
			t_entrenador* entrenador = list_remove_by_condition(listaALaQuePertenece(unEntrenador), esElMismo);

			if (strcmp(ALGORITMO, "FIFO") == 0)
			{
				algoritmoFifo(entrenador);
			}
		}

	}
}

void algoritmoFifo(t_entrenador* entrenador)
{
	t_pokemon* aMoverse = entrenador->pokemonAMoverse;
	uint32_t ciclos = 0;

	if (aMoverse!= NULL)
	{
		uint32_t distancia = distanciaEntrenadorPokemon(entrenador->posicionX, entrenador->posicionY,aMoverse->posicionX,aMoverse->posicionY);
		while (ciclos <= distancia)
		{
			ciclos++;
			sleep(retardoCpu);
		}
		pthread_mutex_lock(&mutexCiclosConsumidos);
		ciclosConsumidos += ciclos; //ACUMULO LOS CICLOS DE CPU CONSUMIDOS
		pthread_mutex_unlock(&mutexCiclosConsumidos);
		//CONEXION AL BROKER Y ENVIO DE MENSAJE CATCH
		catch_pokemon_msg* mensaje = catch_msg(aMoverse->nombre,aMoverse->posicionX,aMoverse->posicionY);
		uint32_t* idMensajeExec = malloc(sizeof(int)); // no se libera aca porque se libera cuando liberamos  ids enviados
		*idMensajeExec = enviar_mensaje(ip_broker,puerto_broker,CATCH_POKEMON,mensaje,true);
		if (*idMensajeExec == -1)
		{
			list_add(entrenador->pokesAtrapados,aMoverse);
			entrenador->pokemonAMoverse = NULL;
			entrenador->motivoBloqueo = MOTIVO_CAUGHT;
		}
		else
		{
			entrenador->posicionX = entrenador->pokemonAMoverse->posicionX;
			entrenador->posicionY = entrenador->pokemonAMoverse->posicionY;
			entrenador->pokemonAMoverse = NULL;
			entrenador->idRecibido = *idMensajeExec;
			entrenador->motivoBloqueo = MOTIVO_CATCH;
			list_add(estado_bloqueado, entrenador);
			list_add(ids_enviados, idMensajeExec);
		}
	}
}




