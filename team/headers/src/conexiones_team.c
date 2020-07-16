#include "../conexiones_team.h"

void cambiarAExit(t_entrenador* entrenador)
{
	entrenador->pokemonAMoverse = NULL;
	entrenador->motivoBloqueo = ESTADO_EXIT;
	pthread_mutex_lock(&mutexEstadoExit);
	list_add(estado_exit,entrenador);
	pthread_mutex_unlock(&mutexEstadoExit);
	log_info(logger,"Cambio del entrenador %d a la cola EXIT.",entrenador->idEntrenador);
}

void cambiarABlockNada(t_entrenador* entrenador)
{
	entrenador->motivoBloqueo = MOTIVO_NADA;
	pthread_mutex_lock(&mutexEstadoBloqueado);
	list_add(estado_bloqueado,entrenador);
	pthread_mutex_unlock(&mutexEstadoBloqueado);
	log_info(logger,"Cambio del entrenador %d a la cola BLOQUEADO, no tiene NADA que hacer.",entrenador->idEntrenador);
}

void cambiarABlockDeadlock(t_entrenador* entrenador)
{
	entrenador->motivoBloqueo = ESPERA_DEADLOCK;
	pthread_mutex_lock(&mutexEstadoBloqueado);
	list_add(estado_bloqueado,entrenador);
	pthread_mutex_unlock(&mutexEstadoBloqueado);
	log_info(logger,"Cambio del entrenador %d a la cola BLOQUEADO, esperando DEADLOCK.",entrenador->idEntrenador);
}

void cambiarEstado(t_entrenador* entrenador){
	//CAMBIA EL ENTRENADOR A EXIT O BLOQUEADO
	if(puedeAtrapar(entrenador))
	{
		list_add(entrenador->pokesAtrapados,(entrenador->pokemonAMoverse)->nombre);
		entrenador->pokemonAMoverse = NULL;

		if(sonIguales(entrenador->pokesObjetivos,entrenador->pokesAtrapados))
		{
			cambiarAExit(entrenador);
		}
		else if (list_size(entrenador->pokesObjetivos) != list_size(entrenador->pokesAtrapados))
		{
			cambiarABlockNada(entrenador);
		}
		else
		{
			cambiarABlockDeadlock(entrenador);
		}
	}
	else if(sonIguales(entrenador->pokesObjetivos,entrenador->pokesAtrapados))
	{
		cambiarAExit(entrenador);
	}
	else
	{
		cambiarABlockDeadlock(entrenador);
	}
}

bool necesitoElMensaje(uint32_t id){

	bool esIgual(uint32_t* unId){
		return *unId == id;
	}

	return list_any_satisfy(ids_enviados,(void*) esIgual);
}

void mostrar_ids(void* id){
	printf("ID %d\n",*(int*)id);
}

void enviar_gets(t_list* objetivos_globales) {

	void enviar(void* especie) {
		/*CREO EL MENSAJE Y LO ENVIO*/
		char* mensaje = ((t_especie*) especie)->especie;
		get_pokemon_msg* a_enviar = get_msg(mensaje);
		uint32_t* id_respuesta = malloc(sizeof(uint32_t));
		*id_respuesta = enviar_mensaje(ip_broker, puerto_broker,GET_POKEMON,a_enviar,0,true);
		ciclosConsumidos++;
		if (*id_respuesta == -1) {
			log_error(logger,"ERROR al enviar el mensaje GET_POKEMON %s.",a_enviar->nombre_pokemon);
			log_info(logger,"OPERACION DEFAULT: No existen locaciones disponibles para el pokemon %s.",a_enviar->nombre_pokemon);
			printf("No existen locaciones disponibles para el pokemon: '%s'\n",a_enviar->nombre_pokemon);
			free(id_respuesta);
		} else {
			/*AGREGO EL ID A LA LISTA DE IDS ENVIADOS*/
			printf("Envio del mensaje GET_POKEMON '%s' exitoso. ID asignado: %d\n",a_enviar->nombre_pokemon,*id_respuesta);
			pthread_mutex_lock(&mutexIdsEnviados);
			list_add(ids_enviados, id_respuesta);
			pthread_mutex_unlock(&mutexIdsEnviados);
		}

		free(a_enviar);
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
		queue_name colaProductor;

		recv(socket_cliente,&colaProductor,sizeof(queue_name),MSG_WAITALL); // RECIBO PRIMERO LA COLA DE PRODUCTOR.

		queue_name colaMensaje;

		appeared_pokemon_msg* mensaje_recibido_appeared = recibir_mensaje(socket_cliente,&id,&colaMensaje);

		if (mensaje_recibido_appeared != NULL) {
			log_info(logger,"Nuevo mensaje APPEARED_POKEMON %s, en la posicion (%d,%d).",mensaje_recibido_appeared->nombre_pokemon,mensaje_recibido_appeared->coordenada_X,mensaje_recibido_appeared->coordenada_Y);
			if (estaEnListaEspecie(mensaje_recibido_appeared->nombre_pokemon,objetivos_posta) && noSuperaElMaximoQuePuedoRecibir(mensaje_recibido_appeared->nombre_pokemon)) {

				pthread_mutex_lock(&mutexPokemonsRecibidosHistoricos);
				agregarAppearedRecibidoALista(pokemons_recibidos_historicos,mensaje_recibido_appeared);
				pthread_mutex_unlock(&mutexPokemonsRecibidosHistoricos);

				pthread_mutex_lock(&mutexPokemonsRecibidos);
				agregarAppearedRecibidoALista(pokemons_recibidos,mensaje_recibido_appeared);
				pthread_mutex_unlock(&mutexPokemonsRecibidos);
			}
		}

		free(mensaje_recibido_appeared->nombre_pokemon);
		free(mensaje_recibido_appeared);
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

				planificacion(estado_ready);

				pthread_mutex_lock(&mutexHayEntrenadorProcesando);
				hayEntrenadorProcesando = false;
				pthread_mutex_unlock(&mutexHayEntrenadorProcesando);
			}
		}
	}
}

void planificacion(t_list* lista)
{
	t_entrenador* entrenador = NULL;
	t_entrenador* entrenadorAMoverse = NULL;
	cambiosDeContexto++;

	if(string_equals_ignore_case(ALGORITMO,"SJF-SD") || string_equals_ignore_case(ALGORITMO,"SJF-CD")){
		pthread_mutex_lock(&mutexEstadoReady);
		entrenador = elDeMenorEstimacion(lista);
		pthread_mutex_unlock(&mutexEstadoReady);
	} else {
		pthread_mutex_lock(&mutexEstadoReady);
		entrenador = (t_entrenador*) list_remove(lista,0);
		pthread_mutex_unlock(&mutexEstadoReady);
	}

	if(entrenador->motivoBloqueo != RESOLVIENDO_DEADLOCK)
	{
		entrenador->posXAMoverse = entrenador->pokemonAMoverse->posicionX;
		entrenador->posYAMoverse = entrenador->pokemonAMoverse->posicionY;
	}
	else
	{
		t_list* losQueTienenElPokemonQueLeFalta = quienesTienenElPokeQueMeFalta(entrenador,estado_bloqueado);
		entrenadorAMoverse = list_get(losQueTienenElPokemonQueLeFalta,0);
		entrenador->posXAMoverse = entrenadorAMoverse->posicionX;
		entrenador->posYAMoverse = entrenadorAMoverse->posicionY;
	}

	if (entrenador->pokemonAMoverse!= NULL && entrenador != NULL){
		moverEntrenador(entrenador,entrenador->pokemonAMoverse->posicionX,entrenador->pokemonAMoverse->posicionY);
	}

	uint32_t distancia = distanciaEntrenadorPokemon(entrenador->posicionX,entrenador->posicionY, entrenador->pokemonAMoverse->posicionX,entrenador->pokemonAMoverse->posicionY);

	if(distancia != 0){
		pthread_mutex_lock(&mutexEstadoReady);
		list_add(lista,entrenador);
		pthread_mutex_unlock(&mutexEstadoReady);
	} else
	{
		if(entrenadorAMoverse != NULL)
		{
			realizarCambio(entrenador,entrenadorAMoverse);
		}
		else
		{
			//CONEXION AL BROKER Y ENVIO DE MENSAJE CATCH
			catch_pokemon_msg* mensaje = catch_msg(entrenador->pokemonAMoverse->nombre,entrenador->pokemonAMoverse->posicionX,entrenador->pokemonAMoverse->posicionY);
			uint32_t* idMensajeExec = malloc(sizeof(int)); // no se libera aca porque se libera cuando liberamos  ids enviados
			*idMensajeExec = enviar_mensaje(ip_broker,puerto_broker,CATCH_POKEMON,mensaje,0,true);
			log_info(logger,"El entrenador %d envió el mensaje CATCH_POKEMON %s, en la posición (%d,%d).",entrenador->idEntrenador,entrenador->pokemonAMoverse->nombre,entrenador->pokemonAMoverse->posicionX,entrenador->pokemonAMoverse->posicionY);
			ciclosConsumidos++;
			cambiosDeContexto++;
			if (*idMensajeExec == -1){
				printf("Falló el envio del mensaje CATCH_POKEMON %s.\n",mensaje->nombre_pokemon);
				log_error(logger,"ERROR al enviar el mensaje CATCH_POKEMON %s.",mensaje->nombre_pokemon);
				log_info(logger,"OPERACION DEFAULT: El entrenador %d capturó al pokemon %s.",entrenador->idEntrenador,mensaje->nombre_pokemon);
				cambiarEstado(entrenador);
				free(idMensajeExec);
			} else {
				printf("Envio mensaje catch %s, posicion: (%d,%d), id: %d.\n",mensaje->nombre_pokemon,mensaje->coordenada_X,mensaje->coordenada_Y,*idMensajeExec);
				entrenador->idRecibido = *idMensajeExec;
				entrenador->motivoBloqueo = ESPERA_CAUGHT;
				list_add(ids_enviados, idMensajeExec);
				pthread_mutex_lock(&mutexEstadoBloqueado);
				list_add(estado_bloqueado, entrenador);
				pthread_mutex_unlock(&mutexEstadoBloqueado);
				log_info(logger,"Cambio del entrenador %d a la cola BLOQUEADO, esperando respueta del mensaje CATCH.",entrenador->idEntrenador);
			}
			free(mensaje);
		}
	}

		//free(mensaje->nombre_pokemon);
}

void pasar_a_ready(){
	while(1){
		t_list* entrenadoresAPlanificar = todosLosEntrenadoresAPlanificar();

		if(list_size(pokemons_recibidos)>0 && list_size(entrenadoresAPlanificar)>0){

			t_entrenador* entrenadorTemporal = entrenadorAReady(entrenadoresAPlanificar,pokemons_recibidos);

			log_info(logger,"El entrenador con id %d paso a la cola READY.", entrenadorTemporal->idEntrenador);

			bool es_el_mismo_entrenador(t_entrenador* unEntrenador){
				return unEntrenador->idEntrenador == entrenadorTemporal->idEntrenador;
			}

			//Aca tengo que poner mutex??
			t_list* lista = listaALaQuePertenece(entrenadorTemporal);

			list_remove_and_destroy_by_condition(lista,(void*) es_el_mismo_entrenador,free);

			uint32_t distancia = distanciaEntrenadorPokemon(entrenadorTemporal->posicionX, entrenadorTemporal->posicionY,(entrenadorTemporal->pokemonAMoverse)->posicionX,(entrenadorTemporal->pokemonAMoverse)->posicionY);

			if(string_equals_ignore_case(ALGORITMO,"SJF-SD") || string_equals_ignore_case(ALGORITMO,"SJF-CD")){
				recalcularEstimacion(entrenadorTemporal,distancia);
			}

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

		list_destroy(entrenadoresAPlanificar);
	}
}

void recibirAppeared() {
	uint32_t idRecibido;

	while(1){

		sem_wait(&semAppeared);
		queue_name colaMensaje;
		appeared_pokemon_msg* mensaje_recibido_appeared = recibir_mensaje(socket_appeared,&idRecibido,&colaMensaje);
		sem_post(&semAppeared);

		if (mensaje_recibido_appeared != NULL) {
			log_info(logger,"Nuevo mensaje recibido: APPEARED_POKEMON %s, en la posicion (%d,%d).",mensaje_recibido_appeared->nombre_pokemon,mensaje_recibido_appeared->coordenada_X,mensaje_recibido_appeared->coordenada_Y);
			if (estaEnLaLista(mensaje_recibido_appeared->nombre_pokemon,objetivos_posta) && noSuperaElMaximoQuePuedoRecibir(mensaje_recibido_appeared->nombre_pokemon)) {

				pthread_mutex_lock(&mutexPokemonsRecibidosHistoricos);
				agregarAppearedRecibidoALista(pokemons_recibidos_historicos,mensaje_recibido_appeared);
				pthread_mutex_unlock(&mutexPokemonsRecibidosHistoricos);

				pthread_mutex_lock(&mutexPokemonsRecibidos);
				agregarAppearedRecibidoALista(pokemons_recibidos,mensaje_recibido_appeared);
				pthread_mutex_unlock(&mutexPokemonsRecibidos);
			}
		} else {
			if(socket_appeared > 0) close(socket_appeared);
			invocarHiloReconexion();
		}
	}
}

void recibirLocalized() { // FALTA TESTEAR AL RECIBIR MENSAJE DE BROKER

	uint32_t id;

	while (1) {

		sem_wait(&semLocalized);
		queue_name colaMensaje;
		localized_pokemon_msg* mensaje_recibido_localized = recibir_mensaje(socket_localized,&id,&colaMensaje); //Devuelve NULL si falla, falta manejar eso para que vuelva a reintentar la conexion.
		sem_post(&semLocalized);

		if (mensaje_recibido_localized != NULL) {
			log_info(logger,"Nuevo mensaje recibido: LOCALIZED_POKEMON %s, en %d posiciones.",mensaje_recibido_localized->nombre_pokemon,mensaje_recibido_localized->cantidad_posiciones);
			if ((estaEnLaLista((mensaje_recibido_localized->nombre_pokemon),objetivos_posta))&&
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
			if(socket_localized > 0) close(socket_localized);
			invocarHiloReconexion();
		}
	}
}

void recibirCaught(){ // FALTA TESTEAR AL RECIBIR MENSAJE DE BROKER

	uint32_t idRecibido;

	while(1){

		sem_wait(&semCaught);
		queue_name colaMensaje;
		caught_pokemon_msg* mensaje_recibido = recibir_mensaje(socket_caught,&idRecibido,&colaMensaje);
		sem_post(&semCaught);

		if(mensaje_recibido != NULL){ //Verifico si recibo el mensaje.
			if(mensaje_recibido->resultado){log_info(logger,"Nuevo mensaje recibido: CAUGHT_POKEMON. Resultado: Atrapado.");}else{log_info(logger,"Nuevo mensaje recibido: CAUGHT_POKEMON. Resultado: No Atrapado.");}
			//if(mensaje_recibido->resultado){log_info(logger,"Nuevo mensaje recibido: CAUGHT_POKEMON. Resultado: SI");}else{log_info(logger,"Nuevo mensaje recibido: CAUGHT_POKEMON. Resultado: NO");}
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
			if(socket_caught > 0) close(socket_caught);
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

			printf("\nFalló la conexion con el Broker. Intento de reconexion numero %d...\n", intento);
			log_error(logger,"Falló la conexion con el Broker. Intento de reconexion numero %d...", intento);
			sleep(tiempo_reconexion);
			intento++;
		} else {

			printf("Suscripciones exitosas!\n\n");
			log_info(logger,"Suscripciones con el broker exitosas!");
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
	t_entrenador* entrenador;
	bool logueo = false;
	uint32_t cantidadDeadlocks = 0;
	while(1)
	{
		printf("\nChequeando si hay deadlock... \n");

		if(list_is_empty(estado_ready) && list_is_empty(estado_new) && !hayEntrenadorProcesando && list_all_satisfy(estado_bloqueado,(void*) bloqueadoPorDeadlock) && !list_is_empty(estado_bloqueado))
		{

			if(!logueo) // se hace esto para que aparezca el inicio de correccion del deadlock 1 sola vez y no se loguee cada vez que entramos aca
			{
				logueo = true;
				log_info(logger,"Inicia el algoritmo de correccion de DEADLOCK.");
			}

			if(!list_is_empty(estado_bloqueado)){

				entrenador = list_remove(estado_bloqueado,0);
				cambiosDeContexto++;

				t_list* losQueTienenElPokemonQueLeFalta = quienesTienenElPokeQueMeFalta(entrenador,estado_bloqueado);

				t_entrenador* entrenadorAMoverse = list_get(losQueTienenElPokemonQueLeFalta,0);

				if(entrenadorAMoverse == NULL){
					cambiarEstado(entrenador);
					if(list_is_empty(estado_bloqueado)){break;}
				}
				else
				{
					printf("Hay deadlock. Solucionando deadlock... \n\n");
					entrenador->motivoBloqueo = RESOLVIENDO_DEADLOCK;
					resolverDeadlock();
					cantidadDeadlocks++;
				}

				list_destroy(losQueTienenElPokemonQueLeFalta);
			}
		} else {
			printf("No hay deadlock.\n\n");
			if(list_is_empty(estado_ready) && list_is_empty(estado_new) && !hayEntrenadorProcesando && list_is_empty(estado_bloqueado)){break;}
		}

		sleep(3); // con esto dejo el proceso corriendo y chequeo
	}

	log_info(logger,"Finaliza el algoritmo de correccion de DEADLOCK.");
	log_info(logger,"Se cumplio el OBJETIVO del TEAM.");

	printf("\nLos entrenadores fueron planificados en su totalidad.\n\n");

	list_iterate(estado_exit,(void*)mostrarCiclos);
	printf("\nLa cantidad de deadlocks producidos y resueltos es: %d\n", cantidadDeadlocks);
	printf("\nLa cantidad de ciclos de CPU totales consumidos es: %d\n",ciclosConsumidos);
	printf("\nLa cantidad de cambios de contexto realizados es: %d\n\n",cambiosDeContexto);
}

void resolverDeadlock()
{
	while (1)
	{
		if (!list_is_empty(estado_ready))
		{
			if(!hayEntrenadorProcesando)
			{
				pthread_mutex_lock(&mutexHayEntrenadorProcesando);
				hayEntrenadorProcesando = true;
				pthread_mutex_unlock(&mutexHayEntrenadorProcesando);

				planificacion(estado_bloqueado);

				pthread_mutex_lock(&mutexHayEntrenadorProcesando);
				hayEntrenadorProcesando = false;
				pthread_mutex_unlock(&mutexHayEntrenadorProcesando);
			}
		}
	}
}


