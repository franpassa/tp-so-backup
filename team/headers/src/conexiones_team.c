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
	log_info(logger,"Cambio del entrenador %d a la cola BLOQUEADO, no tiene NADA que hacer.",entrenador->idEntrenador);
	pthread_mutex_lock(&mutexEstadoBloqueado);
	list_add(estado_bloqueado,entrenador);
	pthread_mutex_unlock(&mutexEstadoBloqueado);
	sem_post(&semEntrenadoresAPlanificar);
}

void cambiarABlockDeadlock(t_entrenador* entrenador)
{
	entrenador->motivoBloqueo = ESPERA_DEADLOCK;
	log_info(logger,"Cambio del entrenador %d a la cola BLOQUEADO, esperando DEADLOCK.",entrenador->idEntrenador);
	pthread_mutex_lock(&mutexEstadoBloqueado);
	list_add(estado_bloqueado,entrenador);
	pthread_mutex_unlock(&mutexEstadoBloqueado);
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
	else if(entrenador->motivoBloqueo == RESOLVIENDO_DEADLOCK)
	{
		pthread_mutex_lock(&mutexEstadoReady);
		list_add(estado_ready,entrenador);
		pthread_mutex_unlock(&mutexEstadoReady);
		sem_post(&semEstadoExec);
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

void enviar_gets(t_list* objetivos_globales){

	void enviar(void* especie) {
		/*CREO EL MENSAJE Y LO ENVIO*/
		char* mensaje = ((t_especie*) especie)->especie;
		get_pokemon_msg* a_enviar = get_msg(mensaje);
		uint32_t* id_respuesta = malloc(sizeof(uint32_t));
		*id_respuesta = enviar_mensaje(ip_broker, puerto_broker,GET_POKEMON,a_enviar,0,true);
		pthread_mutex_lock(&mutexCiclosConsumidos);
		ciclosConsumidos++;
		pthread_mutex_unlock(&mutexCiclosConsumidos);
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

void esperar_cliente(int* socket_servidor){

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

		uint32_t mi_socket;

		appeared_pokemon_msg* mensaje_recibido_appeared = recibir_mensaje(socket_cliente,&id,&colaMensaje,&mi_socket);

		if (mensaje_recibido_appeared != NULL) {
			log_info(logger,"Nuevo mensaje APPEARED_POKEMON %s, en la posicion (%d,%d).",mensaje_recibido_appeared->nombre_pokemon,mensaje_recibido_appeared->coordenada_X,mensaje_recibido_appeared->coordenada_Y);
			if (estaEnListaEspecie(mensaje_recibido_appeared->nombre_pokemon,objetivos_posta) && noSuperaElMaximoQuePuedoRecibir(mensaje_recibido_appeared->nombre_pokemon)) {

				pthread_mutex_lock(&mutexPokemonsRecibidos);
				agregarAppearedRecibidoALista(pokemons_recibidos,mensaje_recibido_appeared);
				pthread_mutex_unlock(&mutexPokemonsRecibidos);

				sem_post(&semPokemonsRecibidos);

				pthread_mutex_lock(&mutexPokemonsRecibidosHistoricos);
				agregarAppearedRecibidoALista(pokemons_recibidos_historicos,mensaje_recibido_appeared);
				pthread_mutex_unlock(&mutexPokemonsRecibidosHistoricos);
			}
		}

		free(mensaje_recibido_appeared->nombre_pokemon);
		free(mensaje_recibido_appeared);
	}
}

void estado_exec(){
	while (1){

		sem_wait(&semEstadoExec);

		if(!hayEntrenadorProcesando){

			pthread_mutex_lock(&mutexHayEntrenadorProcesando);
			hayEntrenadorProcesando = true;
			pthread_mutex_unlock(&mutexHayEntrenadorProcesando);

			planificacion();

			pthread_mutex_lock(&mutexHayEntrenadorProcesando);
			hayEntrenadorProcesando = false;
			pthread_mutex_unlock(&mutexHayEntrenadorProcesando);
		}
	}
}

void planificacion()
{
	t_entrenador* entrenador = NULL;
	t_entrenador* entrenadorAMoverse = NULL;

	pthread_mutex_lock(&mutexCambiosDeContexto);
	cambiosDeContexto++;
	pthread_mutex_unlock(&mutexCambiosDeContexto);

	if(string_equals_ignore_case(ALGORITMO,"SJF-SD") || string_equals_ignore_case(ALGORITMO,"SJF-CD")){
		pthread_mutex_lock(&mutexEstadoReady);
		entrenador = elDeMenorEstimacion(estado_ready);
		pthread_mutex_unlock(&mutexEstadoReady);
	} else {
		pthread_mutex_lock(&mutexEstadoReady);
		entrenador = (t_entrenador*) list_remove(estado_ready,0);
		pthread_mutex_unlock(&mutexEstadoReady);
	}

	if(entrenador->motivoBloqueo == RESOLVIENDO_DEADLOCK)
	{
		pthread_mutex_lock(&mutexEstadoBloqueado);
		t_list* losQueTienenElPokemonQueLeFalta = quienesTienenElPokeQueMeFalta(entrenador,estado_bloqueado);
		pthread_mutex_unlock(&mutexEstadoBloqueado);
		entrenadorAMoverse = list_get(losQueTienenElPokemonQueLeFalta,0);
		entrenador->posXAMoverse = entrenadorAMoverse->posicionX;
		entrenador->posYAMoverse = entrenadorAMoverse->posicionY;
	}

	moverEntrenador(entrenador,entrenador->posXAMoverse,entrenador->posYAMoverse);

	uint32_t distanciaRecorrida = distanciaEntrenadorPokemon(entrenador->posicionX,entrenador->posicionY, entrenador->posXAMoverse,entrenador->posYAMoverse);

	if(distanciaRecorrida != 0){
		pthread_mutex_lock(&mutexCambiosDeContexto);
		cambiosDeContexto++;
		pthread_mutex_unlock(&mutexCambiosDeContexto);
		pthread_mutex_lock(&mutexEstadoReady);
		list_add(estado_ready,entrenador);
		pthread_mutex_unlock(&mutexEstadoReady);

		sem_post(&semEstadoExec);
	} else {
		if(entrenador->motivoBloqueo == RESOLVIENDO_DEADLOCK){

			realizarCambio(entrenador,entrenadorAMoverse);

			pthread_mutex_lock(&mutexCiclosConsumidos);
			ciclosConsumidos+=5;
			pthread_mutex_unlock(&mutexCiclosConsumidos);

			entrenador->ciclosAcumulados+=5;

			cambiarEstado(entrenador);

			pthread_mutex_lock(&mutexCambiosDeContexto);
			cambiosDeContexto++;
			pthread_mutex_unlock(&mutexCambiosDeContexto);

			bool mismo(t_entrenador* unEntrenador){
				return unEntrenador->idEntrenador == entrenadorAMoverse->idEntrenador;
			}

			pthread_mutex_lock(&mutexEstadoBloqueado);
			list_remove_by_condition(estado_bloqueado,(void*) mismo);
			pthread_mutex_unlock(&mutexEstadoBloqueado);

			cambiarEstado(entrenadorAMoverse);
			printf("\nCambio realizado entre el entrenador %d, y el entrenador %d.\n",entrenador->idEntrenador,entrenadorAMoverse->idEntrenador);
		} else {
			//CONEXION AL BROKER Y ENVIO DE MENSAJE CATCH
			catch_pokemon_msg* mensaje = catch_msg(entrenador->pokemonAMoverse->nombre,entrenador->pokemonAMoverse->posicionX,entrenador->pokemonAMoverse->posicionY);
			uint32_t* idMensajeExec = malloc(sizeof(int)); // no se libera aca porque se libera cuando liberamos  ids enviados
			*idMensajeExec = enviar_mensaje(ip_broker,puerto_broker,CATCH_POKEMON,mensaje,0,true);
			log_info(logger,"El entrenador %d envió el mensaje CATCH_POKEMON %s, en la posición (%d,%d).",entrenador->idEntrenador,entrenador->pokemonAMoverse->nombre,entrenador->pokemonAMoverse->posicionX,entrenador->pokemonAMoverse->posicionY);
			pthread_mutex_lock(&mutexCiclosConsumidos);
			ciclosConsumidos++;
			pthread_mutex_unlock(&mutexCiclosConsumidos);
			pthread_mutex_lock(&mutexCambiosDeContexto);
			cambiosDeContexto++;
			pthread_mutex_unlock(&mutexCambiosDeContexto);
			if(*idMensajeExec == -1){
				printf("Falló el envio del mensaje CATCH_POKEMON %s.\n",mensaje->nombre_pokemon);
				log_error(logger,"ERROR al enviar el mensaje CATCH_POKEMON %s.",mensaje->nombre_pokemon);
				log_info(logger,"OPERACION DEFAULT: El entrenador %d capturó al pokemon %s.",entrenador->idEntrenador,mensaje->nombre_pokemon);
				cambiarEstado(entrenador);
				free(idMensajeExec);
			} else {
				printf("Envio mensaje catch %s, posicion: (%d,%d), id: %d.\n",mensaje->nombre_pokemon,mensaje->coordenada_X,mensaje->coordenada_Y,*idMensajeExec);
				entrenador->idRecibido = *idMensajeExec;
				entrenador->motivoBloqueo = ESPERA_CAUGHT;
				pthread_mutex_lock(&mutexIdsEnviados);
				list_add(ids_enviados, idMensajeExec);
				pthread_mutex_unlock(&mutexIdsEnviados);
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

		sem_wait(&semPokemonsRecibidos);
		sem_wait(&semEntrenadoresAPlanificar);

		t_list* entrenadoresAPlanificar = todosLosEntrenadoresAPlanificar();

		list_iterate(pokemons_recibidos,mostrarPokemon);

		pthread_mutex_lock(&mutexPokemonsRecibidos);
		t_entrenador* entrenadorTemporal = entrenadorAReady(entrenadoresAPlanificar,pokemons_recibidos);
		pthread_mutex_unlock(&mutexPokemonsRecibidos);

		entrenadorTemporal->posXAMoverse = (entrenadorTemporal->pokemonAMoverse)->posicionX;
		entrenadorTemporal->posYAMoverse = (entrenadorTemporal->pokemonAMoverse)->posicionY;

		if(string_equals_ignore_case(ALGORITMO,"SJF-SD") || string_equals_ignore_case(ALGORITMO,"SJF-CD")){
			uint32_t distancia = distanciaEntrenadorPokemon(entrenadorTemporal->posicionX, entrenadorTemporal->posicionY,entrenadorTemporal->posXAMoverse,entrenadorTemporal->posYAMoverse);

			if(entrenadorTemporal->yaPasoAReadyUnaVez){
				recalcularEstimacion(entrenadorTemporal,distancia);
			} else {
				entrenadorTemporal->yaPasoAReadyUnaVez = true;
			}
		}

		bool es_el_mismo_entrenador(t_entrenador* unEntrenador){
			return unEntrenador->idEntrenador == entrenadorTemporal->idEntrenador;
		}

		pthread_mutex_t mutexLista;
		t_list* lista = listaALaQuePertenece(entrenadorTemporal, &mutexLista);

		pthread_mutex_lock(&mutexLista);
		list_remove_by_condition(lista,(void*) es_el_mismo_entrenador);
		pthread_mutex_unlock(&mutexLista);

		pthread_mutex_lock(&mutexEstadoReady);
		list_add(estado_ready,entrenadorTemporal);
		pthread_mutex_unlock(&mutexEstadoReady);

		sem_post(&semEstadoExec);

		log_info(logger,"El entrenador con id %d paso a la cola READY.", entrenadorTemporal->idEntrenador);

		bool es_el_mismo_pokemon(t_pokemon* pokemon){
			return string_equals_ignore_case(pokemon->nombre, (entrenadorTemporal->pokemonAMoverse)->nombre);
		}

		pthread_mutex_lock(&mutexPokemonsRecibidos);
		list_remove_by_condition(pokemons_recibidos,(void*) es_el_mismo_pokemon);
		pthread_mutex_unlock(&mutexPokemonsRecibidos);

		list_destroy(entrenadoresAPlanificar);
	}
}

void recibirAppeared(){
	uint32_t idRecibido;
	queue_name colaMensaje;
	uint32_t mi_socket;

	while(1){

		appeared_pokemon_msg* mensaje_recibido_appeared = recibir_mensaje(socket_appeared,&idRecibido,&colaMensaje,&mi_socket);

		if (mensaje_recibido_appeared != NULL) {
			log_info(logger,"Nuevo mensaje recibido: APPEARED_POKEMON %s, en la posicion (%d,%d).",mensaje_recibido_appeared->nombre_pokemon,mensaje_recibido_appeared->coordenada_X,mensaje_recibido_appeared->coordenada_Y);
			if (estaEnLaLista(mensaje_recibido_appeared->nombre_pokemon,objetivos_posta) && noSuperaElMaximoQuePuedoRecibir(mensaje_recibido_appeared->nombre_pokemon)) {

				pthread_mutex_lock(&mutexPokemonsRecibidos);
				agregarAppearedRecibidoALista(pokemons_recibidos,mensaje_recibido_appeared);
				pthread_mutex_unlock(&mutexPokemonsRecibidos);

				pthread_mutex_lock(&mutexPokemonsRecibidosHistoricos);
				agregarAppearedRecibidoALista(pokemons_recibidos_historicos,mensaje_recibido_appeared);
				pthread_mutex_unlock(&mutexPokemonsRecibidosHistoricos);

				if(!list_is_empty(pokemons_recibidos)){
					sem_post(&semPokemonsRecibidos);
				}
			}
		} else {
			if(pthread_mutex_trylock(&mutexReconexion)==0){
				suscripcion_a_colas();
				pthread_cond_broadcast(&cond_reconectado);
				pthread_mutex_unlock(&mutexReconexion);
			} else {
				pthread_mutex_lock(&mutexEspera);
				pthread_cond_wait(&cond_reconectado, &mutexEspera);
				pthread_mutex_unlock(&mutexEspera);
			}
		}
	}
}

void recibirLocalized(){ // FALTA TESTEAR AL RECIBIR MENSAJE DE BROKER
	uint32_t id;
	queue_name colaMensaje;
	uint32_t mi_socket;

	while (1) {

		localized_pokemon_msg* mensaje_recibido_localized = recibir_mensaje(socket_localized,&id,&colaMensaje,&mi_socket); //Devuelve NULL si falla, falta manejar eso para que vuelva a reintentar la conexion.

		if (mensaje_recibido_localized != NULL) {
			log_info(logger,"Nuevo mensaje recibido: LOCALIZED_POKEMON %s, en %d posiciones.",mensaje_recibido_localized->nombre_pokemon,mensaje_recibido_localized->cantidad_posiciones);
			if ((estaEnLaLista((mensaje_recibido_localized->nombre_pokemon),objetivos_posta))&&
					(!(estaEnLaLista((mensaje_recibido_localized->nombre_pokemon),pokemons_recibidos_historicos)))&&
					necesitoElMensaje(mensaje_recibido_localized->id_correlativo)){

				pthread_mutex_lock(&mutexPokemonsRecibidos);
				agregarLocalizedRecibidoALista(pokemons_recibidos,mensaje_recibido_localized);
				pthread_mutex_unlock(&mutexPokemonsRecibidos);

				pthread_mutex_lock(&mutexPokemonsRecibidosHistoricos);
				agregarLocalizedRecibidoALista(pokemons_recibidos_historicos,mensaje_recibido_localized);
				pthread_mutex_unlock(&mutexPokemonsRecibidosHistoricos);

				if(!list_is_empty(pokemons_recibidos)){
					sem_post(&semPokemonsRecibidos);
				}
			}
		} else {
			if(pthread_mutex_trylock(&mutexReconexion)==0){
				suscripcion_a_colas();
				pthread_cond_broadcast(&cond_reconectado);
				pthread_mutex_unlock(&mutexReconexion);
			} else {
				pthread_mutex_lock(&mutexEspera);
				pthread_cond_wait(&cond_reconectado, &mutexEspera);
				pthread_mutex_unlock(&mutexEspera);
			}
		}
	}
}

void recibirCaught(){ // FALTA TESTEAR AL RECIBIR MENSAJE DE BROKER
	uint32_t idRecibido;
	queue_name colaMensaje;
	uint32_t mi_socket;

	while(1){

		caught_pokemon_msg* mensaje_recibido = recibir_mensaje(socket_caught,&idRecibido,&colaMensaje,&mi_socket);

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
			if(pthread_mutex_trylock(&mutexReconexion)==0){
				suscripcion_a_colas();
				pthread_cond_broadcast(&cond_reconectado);
				pthread_mutex_unlock(&mutexReconexion);
			} else {
				pthread_mutex_lock(&mutexEspera);
				pthread_cond_wait(&cond_reconectado, &mutexEspera);
				pthread_mutex_unlock(&mutexEspera);
			}
		}
	}
}

void* buscarEntrenador(uint32_t id){

	bool tieneId(void* unEntrenador){
		return ((t_entrenador*) unEntrenador)->idRecibido == id;
	}

	return list_find(estado_bloqueado, tieneId);
}

void iniciar_hilos_escucha(){
	suscripcion_a_colas();

	pthread_create(&hilo_recibir_localized, NULL, (void*) recibirLocalized, NULL);
	pthread_detach(hilo_recibir_localized);
	pthread_create(&hilo_recibir_caught, NULL, (void*) recibirCaught, NULL);
	pthread_detach(hilo_recibir_caught);
	pthread_create(&hilo_recibir_appeared, NULL, (void*) recibirAppeared, NULL);
	pthread_detach(hilo_recibir_appeared);
}

void suscripcion_a_colas(){

	bool conexion_exitosa = false;

	while(!conexion_exitosa){
		socket_appeared = suscribirse_a_cola(APPEARED_POKEMON, ip_broker, puerto_broker);
		socket_localized = suscribirse_a_cola(LOCALIZED_POKEMON, ip_broker, puerto_broker);
		socket_caught = suscribirse_a_cola(CAUGHT_POKEMON, ip_broker, puerto_broker);

		if(socket_appeared != -1 && socket_appeared != -1 && socket_appeared != -1){
			printf("Conexión con el Broker exitosa.\n\n");
			conexion_exitosa = true;
		} else {
			printf("Falló la conexión con el Broker, reintentando en %d segundos...\n", tiempo_reconexion);
			sleep(tiempo_reconexion);
		}
	}
}

void deadlock()
{
	t_entrenador* entrenador;
	bool logueo = false;

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

			if(!list_is_empty(estado_bloqueado))
			{
				if(string_equals_ignore_case(ALGORITMO,"SJF-SD") || string_equals_ignore_case(ALGORITMO,"SJF-SD")){

					bool esElDeMenor(t_entrenador* unEntrenador){
						return esElDeMenorEstimacion(estado_bloqueado,unEntrenador);
					}

					pthread_mutex_lock(&mutexEstadoBloqueado);
					entrenador = list_remove_by_condition(estado_bloqueado,(void*) esElDeMenor);
					pthread_mutex_unlock(&mutexEstadoBloqueado);

				} else {
					pthread_mutex_lock(&mutexEstadoBloqueado);
					entrenador = list_remove(estado_bloqueado,0);
					pthread_mutex_unlock(&mutexEstadoBloqueado);
				}

				t_list* losQueTienenElPokemonQueLeFalta = quienesTienenElPokeQueMeFalta(entrenador,estado_bloqueado);
				t_entrenador* entrenadorAMoverse = list_get(losQueTienenElPokemonQueLeFalta,0);
				if(entrenadorAMoverse == NULL)
				{
					cambiarEstado(entrenador);
					if(list_is_empty(estado_bloqueado))break;
				}
				else
				{
					printf("Hay deadlock. Solucionando deadlock... \n\n");
					entrenador->motivoBloqueo = RESOLVIENDO_DEADLOCK;
					cambiarEstado(entrenador);
					pthread_mutex_lock(&mutexCantidadDeadlocks);
					cantidadDeadlocks++;
					pthread_mutex_unlock(&mutexCantidadDeadlocks);
				}

				list_destroy(losQueTienenElPokemonQueLeFalta);
			}
		} else {
			printf("No hay deadlock.\n\n");
			if(list_is_empty(estado_ready) && list_is_empty(estado_new) && !hayEntrenadorProcesando && list_is_empty(estado_bloqueado))break;
		}

		int tiempo = config_get_int_value(config,"TIEMPO_DEADLOCK");
		sleep(tiempo); // con esto dejo el proceso corriendo y chequeo
	}

	log_info(logger,"Finaliza el algoritmo de correccion de DEADLOCK.");
	log_info(logger,"Se cumplio el OBJETIVO del TEAM.");

	printf("\nLos entrenadores fueron planificados en su totalidad.\n\n");

	list_iterate(estado_exit,(void*)mostrarCiclos);
	printf("\nLa cantidad de deadlocks producidos y resueltos es: %d\n", cantidadDeadlocks);
	printf("\nLa cantidad de ciclos de CPU totales consumidos es: %d\n",ciclosConsumidos);
	printf("\nLa cantidad de cambios de contexto realizados es: %d\n\n",cambiosDeContexto);
}


