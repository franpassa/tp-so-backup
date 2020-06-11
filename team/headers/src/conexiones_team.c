#include "../conexiones_team.h"

void cambiarEstado(t_entrenador* entrenador){
	//CAMBIA EL ENTRENADOR A EXIT O BLOQUEADO
	if(puedeAtrapar(entrenador)){
		list_add(entrenador->pokesAtrapados,entrenador->pokemonAMoverse);
		entrenador->pokemonAMoverse = NULL;

		if(sonIguales(entrenador->pokesAtrapados,entrenador->pokesObjetivos)){
			list_add(estado_exit,entrenador);
		}
		else if (list_size(entrenador->pokesObjetivos) != list_size(entrenador->pokesAtrapados)){
			entrenador->motivoBloqueo = MOTIVO_NADA;
			pthread_mutex_lock(&mutexEstadoBloqueado);
			list_add(estado_bloqueado,entrenador);
			pthread_mutex_unlock(&mutexEstadoBloqueado);
		}
		else {
			entrenador->motivoBloqueo = ESPERA_DEADLOCK;
			pthread_mutex_lock(&mutexEstadoBloqueado);
			list_add(estado_bloqueado,entrenador);
			pthread_mutex_unlock(&mutexEstadoBloqueado);
		}

	} else if(sonIguales(entrenador->pokesAtrapados,entrenador->pokesObjetivos)){
		entrenador->pokemonAMoverse = NULL;
		list_add(estado_exit,entrenador);
	} else {
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
			printf("No existen locaciones disponibles para el pokemon: '%s'\n",a_enviar->nombre_pokemon);
		} else {
			/*AGREGO EL ID A LA LISTA DE IDS ENVIADOS*/
			printf("Envio del mensaje GET_POKEMON '%s' exitoso. ID asignado: %d\n",a_enviar->nombre_pokemon,*id_respuesta);
			pthread_mutex_lock(&mutexIdsEnviados);
			list_add(ids_enviados, id_respuesta);
			pthread_mutex_unlock(&mutexIdsEnviados);
		}
		sleep(1);
	}

	list_iterate(objetivos_globales, enviar);
}

void esperar_cliente(int* socket_servidor) {

	uint32_t id;

	while (1) {

		struct sockaddr_in dir_cliente;

		int tam_direccion = sizeof(struct sockaddr_in);

		//printf("%d\n",*socket_servidor);
		int socket_cliente = accept(*socket_servidor, (void*) &dir_cliente,(socklen_t*) &tam_direccion);
		printf("Nuevo cliente entrante: %d\n", socket_cliente);

		queue_name cola;

		recv(socket_cliente,&cola,sizeof(queue_name),MSG_WAITALL);

		appeared_pokemon_msg* mensaje_recibido_appeared = recibir_mensaje(socket_cliente,&id);

		if (mensaje_recibido_appeared != NULL) {

			if ((estaEnLaLista((mensaje_recibido_appeared->nombre_pokemon),objetivos_globales)) && (!(estaEnLaLista((mensaje_recibido_appeared->nombre_pokemon),pokemons_recibidos_historicos)))) {

				pthread_mutex_lock(&mutexPokemonsRecibidosHistoricos);
				agregarAppearedRecibidoALista(pokemons_recibidos_historicos,mensaje_recibido_appeared);
				pthread_mutex_unlock(&mutexPokemonsRecibidosHistoricos);

				pthread_mutex_lock(&mutexPokemonsRecibidos);
				agregarAppearedRecibidoALista(pokemons_recibidos,mensaje_recibido_appeared);
				pthread_mutex_unlock(&mutexPokemonsRecibidos);
			}
		}
	}
}

void estado_exec()
{
	while (1){

		if (!list_is_empty(estado_ready)){

			if(!hayEntrenadorProcesando){

				pthread_mutex_lock(&mutexHayEntrenadorProcesando);
				hayEntrenadorProcesando = true;
				pthread_mutex_unlock(&mutexHayEntrenadorProcesando);

				if (string_equals_ignore_case(ALGORITMO, "FIFO")){

						algoritmoFifo();
				}

				pthread_mutex_lock(&mutexHayEntrenadorProcesando);
				hayEntrenadorProcesando = false;
				pthread_mutex_unlock(&mutexHayEntrenadorProcesando);
			}
		}
	}
}

void algoritmoFifo()
{
	t_entrenador* entrenador = (t_entrenador*) list_get(estado_ready,0);

	bool esElMismo(t_entrenador* unEntrenador){
		return unEntrenador->idEntrenador == entrenador->idEntrenador;
	}

	list_remove_by_condition(listaALaQuePertenece(entrenador),(void*) esElMismo);
	t_pokemon* aMoverse = entrenador->pokemonAMoverse;

	if (aMoverse!= NULL)
	{
		uint32_t distancia = distanciaEntrenadorPokemon(entrenador->posicionX, entrenador->posicionY,aMoverse->posicionX,aMoverse->posicionY);
		moverEntrenador(entrenador,aMoverse->posicionX,aMoverse->posicionY,retardoCpu, logger);
		pthread_mutex_lock(&mutexCiclosConsumidos);
		ciclosConsumidos += distancia; //ACUMULO LOS CICLOS DE CPU CONSUMIDOS
		pthread_mutex_unlock(&mutexCiclosConsumidos);
		pthread_mutex_lock(&mutexLog);
		log_info(logger,"el entrenador %d termino de moverse y esta en la pos %d, %d",entrenador->idEntrenador,entrenador->posicionX,entrenador->posicionY);
		pthread_mutex_unlock(&mutexLog);

		//CONEXION AL BROKER Y ENVIO DE MENSAJE CATCH
		catch_pokemon_msg* mensaje = catch_msg(aMoverse->nombre,aMoverse->posicionX,aMoverse->posicionY);
		uint32_t* idMensajeExec = malloc(sizeof(int)); // no se libera aca porque se libera cuando liberamos  ids enviados
		*idMensajeExec = enviar_mensaje(ip_broker,puerto_broker,CATCH_POKEMON,mensaje,true);

		if (*idMensajeExec == -1)
		{
			printf("Falló el envio del mensaje CATCH_POKEMON %s.\n",mensaje->nombre_pokemon);
			cambiarEstado(entrenador);
		} else {
			printf("Envio mensaje catch %s, posicion: (%d,%d), id: %d.\n",mensaje->nombre_pokemon,mensaje->coordenada_X,mensaje->coordenada_Y,*idMensajeExec);
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
	while(1){
		if(list_size(pokemons_recibidos)>0){
			t_list* listaAPlanificar = todosLosEntrenadoresAPlanificar();

			pthread_mutex_lock(&mutexPokemonsRecibidos);
			t_entrenador* entrenadorTemporal = entrenadorAReady(listaAPlanificar,pokemons_recibidos);
			pthread_mutex_unlock(&mutexPokemonsRecibidos);

			pthread_mutex_lock(&mutexLog);
			log_info(logger,"el entrenador con id %d paso a la cola ready", entrenadorTemporal->idEntrenador);
			pthread_mutex_unlock(&mutexLog);

			bool es_el_mismo_entrenador(t_entrenador* unEntrenador){
				return unEntrenador->idEntrenador == entrenadorTemporal->idEntrenador;
			}

			//Aca tengo que poner mutex??
			t_list* lista = listaALaQuePertenece(entrenadorTemporal);
			list_remove_by_condition(lista,(void*) es_el_mismo_entrenador);

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

void recibirAppeared() {
	appeared_pokemon_msg* mensaje_recibido_appeared;

	while(1){

		uint32_t idRecibido;

		sem_wait(&semAppeared);
		mensaje_recibido_appeared = recibir_mensaje(socket_appeared,&idRecibido);
		sem_post(&semAppeared);

		if (mensaje_recibido_appeared != NULL) {
			if ((estaEnLaLista((mensaje_recibido_appeared->nombre_pokemon),objetivos_globales)) && (!(estaEnLaLista((mensaje_recibido_appeared->nombre_pokemon),pokemons_recibidos_historicos)))) {

				pthread_mutex_lock(&mutexPokemonsRecibidosHistoricos);
				agregarAppearedRecibidoALista(pokemons_recibidos_historicos,mensaje_recibido_appeared);
				pthread_mutex_unlock(&mutexPokemonsRecibidosHistoricos);

				pthread_mutex_lock(&mutexPokemonsRecibidos);
				agregarAppearedRecibidoALista(pokemons_recibidos,mensaje_recibido_appeared);
				pthread_mutex_unlock(&mutexPokemonsRecibidos);
			}
		} else {
			close(socket_appeared);
			invocarHiloReconexion();
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

			if ((estaEnLaLista((mensaje_recibido_localized->nombre_pokemon),objetivos_globales))&&
					(!(estaEnLaLista((mensaje_recibido_localized->nombre_pokemon),pokemons_recibidos_historicos)))&&
					necesitoElMensaje(mensaje_recibido_localized->id_correlativo)){

				pthread_mutex_lock(&mutexPokemonsRecibidosHistoricos);
				agregarLocalizedRecibidoALista(pokemons_recibidos_historicos,mensaje_recibido_localized);
				pthread_mutex_unlock(&mutexPokemonsRecibidosHistoricos);

				pthread_mutex_lock(&mutexPokemonsRecibidos);
				agregarLocalizedRecibidoALista(pokemons_recibidos,mensaje_recibido_localized);
				pthread_mutex_unlock(&mutexPokemonsRecibidos);
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

			if(necesitoElMensaje(mensaje_recibido->id_correlativo)){ //Busco el entrenador que mando el mensaje.

				t_entrenador* entrenador = (t_entrenador*) buscarEntrenador(mensaje_recibido->id_correlativo);

				if(mensaje_recibido->resultado == 1){ //Verifico que la respuesta del mensaje sea SI. *****FALTA REVISAR*****

					bool esIgual(uint32_t* unId){
						return *unId == mensaje_recibido->id_correlativo;
					}

					pthread_mutex_lock(&mutexIdsEnviados);
					list_remove_and_destroy_by_condition(ids_enviados,(void*) esIgual, free); // Saco el id de la lista de ids_enviados.
					pthread_mutex_unlock(&mutexIdsEnviados);


					bool esElMismo(t_entrenador* unEntrenador) {
						return unEntrenador->idEntrenador == entrenador->idEntrenador;
					}

					pthread_mutex_lock(&mutexEstadoBloqueado);
					list_remove_by_condition(estado_bloqueado,(void*) esElMismo);
					pthread_mutex_unlock(&mutexEstadoBloqueado);

					cambiarEstado(entrenador);

					if((entrenador->motivoBloqueo == MOTIVO_NADA)&&(list_size(pokemons_recibidos)>0)){

						entrenador->pokemonAMoverse = pokemonMasCercano(entrenador,pokemons_recibidos);

						bool esElMismoPokemon(t_pokemon* pokemon){
								return string_equals_ignore_case(pokemon->nombre,entrenador->pokemonAMoverse->nombre);
							}

						pthread_mutex_lock(&mutexPokemonsRecibidos);
						list_remove_by_condition(pokemons_recibidos,(void*) esElMismoPokemon);
						pthread_mutex_unlock(&mutexPokemonsRecibidos);

						pthread_mutex_lock(&mutexEstadoReady);
						list_add(estado_ready,entrenador);
						pthread_mutex_unlock(&mutexEstadoReady);
					}
				} else {

					bool esIgual(uint32_t* unId){
						return *unId == idRecibido;
					}

					pthread_mutex_lock(&mutexIdsEnviados);
					list_remove_and_destroy_by_condition(ids_enviados,(void*) esIgual, free); // Saco el id de la lista de ids_enviados.
					pthread_mutex_unlock(&mutexIdsEnviados);

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
	sem_wait(&semAppeared);

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

			sem_post(&semCaught);
			sem_post(&semLocalized);
			sem_post(&semAppeared);
			break;
		}
	}

	pthread_mutex_unlock(&mutexReconexion);
}


void deadlock()
{
	while(1)
	{
		printf("Chequeando si hay deadlock. \n");
		if(list_is_empty(estado_ready) && list_is_empty(estado_new) && !hayEntrenadorProcesando && !list_all_satisfy(estado_bloqueado,bloqueadoPorNada))
		{
			printf("Hay deadlock. \n");
			t_entrenador* entrenador =	list_remove(estado_bloqueado,0);
			t_entrenador* entrenadorAMoverse = list_get(quienesTienenElPokeQueMeFalta(entrenador,estado_bloqueado),0);

			if(entrenadorAMoverse == NULL){
				printf("Se resolvió el deadlock.\n");
				break;
			}

			moverEntrenador(entrenador,entrenadorAMoverse->posicionX,entrenadorAMoverse->posicionY,retardoCpu,logger);
			realizarCambio(entrenador,entrenadorAMoverse);
			cambiarEstado(entrenador);
		}
		printf("No hay deadlock. \n");
		sleep(3); // con esto dejo el proceso corriendo y chequeo
	}
}


