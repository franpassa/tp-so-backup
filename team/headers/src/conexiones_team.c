#include "../conexiones_team.h"

void cambiarEstado(t_entrenador* entrenador){
	//CAMBIA EL ENTRENADOR A EXIT O BLOQUEADO
	if(puedeAtrapar(entrenador)){
		list_add(entrenador->pokesAtrapados,entrenador->pokemonAMoverse);

		if(sonIguales(entrenador->pokesAtrapados,entrenador->pokesObjetivos)){
			list_add(estado_exit,entrenador);
		}
		else{
			entrenador->motivoBloqueo = MOTIVO_NADA;
			pthread_mutex_lock(&mutexEstadoBloqueado);
			list_add(estado_bloqueado,entrenador);
			pthread_mutex_unlock(&mutexEstadoBloqueado);
		}

	} else if(!sonIguales(entrenador->pokesAtrapados,entrenador->pokesObjetivos)){
		entrenador->motivoBloqueo = ESPERA_DEADLOCK;
		pthread_mutex_lock(&mutexEstadoBloqueado);
		list_add(estado_bloqueado,entrenador);
		pthread_mutex_unlock(&mutexEstadoBloqueado);
	}
}

bool necesitoElMensaje(uint32_t id){

	bool esIgual(uint32_t* unId){
		return *unId == id;
	}

	return list_any_satisfy(ids_enviados,(void*) esIgual);
}

void enviar_gets(t_list* objetivos_globales) {

	void enviar(void* especie) {
		/*CREO EL MENSAJE Y LO ENVIO*/
		char* mensaje = ((t_especie*) especie)->especie;
		get_pokemon_msg* a_enviar = get_msg(mensaje);
		uint32_t* id_respuesta = malloc(sizeof(uint32_t));
		*id_respuesta = enviar_mensaje(ip_broker, puerto_broker,GET_POKEMON,a_enviar,true);
		if (*id_respuesta == -1) {
			printf("Fallo el envio del mensaje GET_POKEMON '%s'\n",a_enviar->nombre_pokemon);
		} else {
			/*AGREGO EL ID A LA LISTA DE IDS ENVIADOS*/
			printf("Envio del mensaje GET_POKEMON '%s' exitoso. ID asignado: %d\n",a_enviar->nombre_pokemon,*id_respuesta);
			list_add(ids_enviados, id_respuesta);
		}
		sleep(3);
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

void esperar_cliente(int* socket_servidor) {
// ES UN ASCO

	while (1) {

		struct sockaddr_in dir_cliente;

		int tam_direccion = sizeof(struct sockaddr_in);

		//printf("%d\n",*socket_servidor);
		int socket_cliente = accept(*socket_servidor, (void*) &dir_cliente,(socklen_t*) &tam_direccion);
		printf("Nuevo cliente entrante: %d\n", socket_cliente);

		queue_name cola;

		if (recv(socket_cliente, &cola, sizeof(queue_name), MSG_WAITALL) == -1) {
			perror("Error al recibir el mensaje");
			sleep(40);
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
			uint32_t id;
			appeared_pokemon_msg* msg = recibir_mensaje(socket_cliente,&id);
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

			bool esElMismo(t_entrenador* entrenador)
			{
				return entrenador->idEntrenador == unEntrenador->idEntrenador;
			}

			hayEntrenadorProcesando = true;

			list_remove_by_condition(listaALaQuePertenece(unEntrenador),(void*) esElMismo);

			if (string_equals_ignore_case(ALGORITMO, "FIFO"))
			{
				algoritmoFifo(unEntrenador);
			}
		}

		sleep(2);

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

		entrenador->posicionX = aMoverse->posicionX;
		entrenador->posicionY = aMoverse->posicionY;

		//CONEXION AL BROKER Y ENVIO DE MENSAJE CATCH
		catch_pokemon_msg* mensaje = catch_msg(aMoverse->nombre,aMoverse->posicionX,aMoverse->posicionY);
		uint32_t* idMensajeExec = malloc(sizeof(int)); // no se libera aca porque se libera cuando liberamos  ids enviados
		*idMensajeExec = enviar_mensaje(ip_broker,puerto_broker,CATCH_POKEMON,mensaje,true);

		if (*idMensajeExec == -1)
		{
			cambiarEstado(entrenador);
		} else {
			entrenador->idRecibido = *idMensajeExec;
			entrenador->motivoBloqueo = ESPERA_CAUGHT;
			list_add(ids_enviados, idMensajeExec);

			pthread_mutex_lock(&mutexEstadoBloqueado);
			list_add(estado_bloqueado, entrenador);
			pthread_mutex_unlock(&mutexEstadoBloqueado);
		}
	}
}

void pasar_a_ready(){
	t_entrenador* entrenadorTemporal;
	t_list* listaAPlanificar;

	while(1){
		if(list_size(pokemons_recibidos)>0){
			listaAPlanificar = todosLosEntrenadoresAPlanificar();

			pthread_mutex_lock(&mutexPokemonsRecibidos);
			entrenadorTemporal = entrenadorAReady(listaAPlanificar,pokemons_recibidos);
			pthread_mutex_unlock(&mutexPokemonsRecibidos);

			bool es_el_mismo_entrenador(t_entrenador* unEntrenador){
				return unEntrenador->idEntrenador == entrenadorTemporal->idEntrenador;
			}

			//Aca tengo que poner mutex??
			list_remove_by_condition(listaALaQuePertenece(entrenadorTemporal),(void*) es_el_mismo_entrenador);

			pthread_mutex_lock(&mutexEstadoReady);
			list_add(estado_ready,entrenadorTemporal);
			pthread_mutex_unlock(&mutexEstadoReady);

			bool es_el_mismo_pokemon(t_pokemon* pokemon){
				return string_equals_ignore_case(pokemon->nombre, (entrenadorTemporal->pokemonAMoverse)->nombre);
			}

			pthread_mutex_lock(&mutexPokemonsRecibidos);
			list_remove_by_condition(pokemons_recibidos,(void*) es_el_mismo_pokemon);
			pthread_mutex_unlock(&mutexPokemonsRecibidos);
		}
	}
}

void recibirLocalized() { // FALTA TESTEAR AL RECIBIR MENSAJE DE BROKER

	localized_pokemon_msg* mensaje_recibido_localized;

	while (1) {
		uint32_t id;

		sem_wait(&semLocalized);
		mensaje_recibido_localized = recibir_mensaje(socket_localized,&id); //Devuelve NULL si falla, falta manejar eso para que vuelva a reintentar la conexion.
		sem_post(&semLocalized);

		if (mensaje_recibido_localized != NULL) {
				printf("HOLA");
				pthread_mutex_lock(&mutexPokemonsRecibidosHistoricos);
			if ((estaEnLaLista((mensaje_recibido_localized->nombre_pokemon),objetivos_globales)) && (!(estaEnLaLista((mensaje_recibido_localized->nombre_pokemon),pokemons_recibidos_historicos)))) {
				agregarPokemonsRecibidosALista(pokemons_recibidos_historicos,mensaje_recibido_localized);
				pthread_mutex_unlock(&mutexPokemonsRecibidosHistoricos);
				t_list* todosLosEntrenadores = todosLosEntrenadoresAPlanificar();
				pthread_mutex_lock(&mutexPokemonsRecibidos);
				agregarPokemonsRecibidosALista(pokemons_recibidos,mensaje_recibido_localized);
				t_entrenador* entrenadorReady = entrenadorAReady(todosLosEntrenadores, pokemons_recibidos);
				pthread_mutex_unlock(&mutexPokemonsRecibidos);

				bool esElMismo(t_entrenador* entrenador) {
					return entrenador->idEntrenador == entrenadorReady->idEntrenador;
				}

				pthread_mutex_lock(&mutexEstadoReady);
				list_add(estado_ready,entrenadorAReady);
				pthread_mutex_unlock(&mutexEstadoReady);

				list_remove_by_condition(listaALaQuePertenece(entrenadorReady),(void*) esElMismo);
			}
		} else {
			close(socket_localized);
			invocarHiloReconexion();
		}
	}
}

void recibirCaught(){ // FALTA TESTEAR AL RECIBIR MENSAJE DE BROKER

	caught_pokemon_msg* mensaje_recibido;

	while(1){

		uint32_t idRecibido;

		sem_wait(&semCaught);
		mensaje_recibido = recibir_mensaje(socket_caught,&idRecibido);
		sem_post(&semCaught);

		if(mensaje_recibido != NULL){ //Verifico si recibo el mensaje.

			printf("HOLA");

			if(necesitoElMensaje(idRecibido)){ //Busco el entrenador que mando el mensaje.

				t_entrenador* entrenador = (t_entrenador*) buscarEntrenador(idRecibido);

				if(mensaje_recibido->resultado == 1){ //Verifico que la respuesta del mensaje sea SI. *****FALTA REVISAR*****

					bool esIgual(uint32_t* unId){
						return *unId == idRecibido;
					}

					pthread_mutex_lock(&mutexIdsEnviados);
					list_remove_by_condition(ids_enviados,(void*) esIgual); // Saco el id de la lista de ids_enviados.
					pthread_mutex_unlock(&mutexIdsEnviados);


					bool esElMismo(t_entrenador* unEntrenador) {
						return unEntrenador->idEntrenador == entrenador->idEntrenador;
					}

					pthread_mutex_lock(&mutexEstadoBloqueado);
					list_remove_by_condition(estado_bloqueado,(void*) esElMismo);
					pthread_mutex_unlock(&mutexEstadoBloqueado);

					cambiarEstado(entrenador);

					if((entrenador->motivoBloqueo == MOTIVO_NADA)&&(list_size(pokemons_recibidos)>0)&&(list_size(estado_ready)==0)){
						entrenador->pokemonAMoverse = pokemonMasCercano(entrenador,pokemons_recibidos);
						pthread_mutex_lock(&mutexEstadoReady);
						list_add(estado_ready,entrenador);
						pthread_mutex_unlock(&mutexEstadoReady);

						bool esElMismoPokemon(t_pokemon* pokemon){
							return string_equals_ignore_case(pokemon->nombre,entrenador->pokemonAMoverse->nombre);
						}

						pthread_mutex_lock(&mutexPokemonsRecibidos);
						list_remove_by_condition(pokemons_recibidos,(void*) esElMismoPokemon);
						pthread_mutex_unlock(&mutexPokemonsRecibidos);
					}
				} else {
					entrenador->pokemonAMoverse = NULL;
					entrenador->idRecibido = 0;
					entrenador->motivoBloqueo = MOTIVO_NADA;
				}
			}
		} else {
			close(socket_caught);
			invocarHiloReconexion();
		}
	}
}

void* buscarEntrenador(uint32_t id){

	bool tieneId(void* unEntrenador){
		return ((t_entrenador*) unEntrenador)->idRecibido == id;
	}

	return list_find(estado_bloqueado, tieneId);
}

void invocarHiloReconexion(){
	pthread_t hiloReconexion;

	pthread_create(&hiloReconexion, NULL,(void*) conectarABroker, NULL);

	pthread_detach(hiloReconexion);
}

void conectarABroker(){

	if(pthread_mutex_trylock(&mutexReconexion)!=0){
		return;
	}

	int intento = 1;

	sem_wait(&semCaught);
	sem_wait(&semLocalized);
//	sem_wait(&semAppeared);

	while (1){

		socket_caught = suscribirse_a_cola(CAUGHT_POKEMON,ip_broker,puerto_broker);
		socket_localized = suscribirse_a_cola(LOCALIZED_POKEMON,ip_broker,puerto_broker);
		socket_appeared = suscribirse_a_cola(APPEARED_POKEMON,ip_broker,puerto_broker);

		bool conexionExitosa = socket_caught != -1 && socket_localized != -1 && socket_appeared != -1;

		if(!conexionExitosa){
			printf("Falló la conexion con el Broker. Intento de reconexion numero %d...\n\n", intento);
			sleep(tiempo_reconexion);
			intento++;
		} else {

			printf("Suscripciones exitosas!\n\n");

			if(!envioGets){
				enviar_gets(objetivos_globales);
				envioGets = true;
			}

			sem_post(&semCaught);
			sem_post(&semLocalized);
//			sem_post(&semAppeared);
			break;
		}
	}

	pthread_mutex_unlock(&mutexReconexion);
}



