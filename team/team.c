#include "headers/team.h"

int main()
{
	inicializarPrograma(); //Inicializo logger y config

	char* ip_broker = config_get_string_value(config,"IP_BROKER");
	char* puerto_broker = config_get_string_value(config, "PUERTO_BROKER");
	char** posicionesEntrenadores = config_get_array_value(config,"POSICIONES_ENTRENADORES");
	char** pokesEntrenadores = config_get_array_value(config, "POKEMON_ENTRENADORES");
	char** pokesObjetivos = config_get_array_value(config, "OBJETIVOS_ENTRENADORES");

	estado_new = crearListaDeEntrenadores(posicionesEntrenadores,pokesEntrenadores,pokesObjetivos);
	t_list* pokemons_objetivos = crearListaPokesObjetivos(estado_new);
	t_list* objetivos_globales = crearListaObjetivoGlobal(pokemons_objetivos);

	int socket_escucha = iniciar_servidor(IP,PUERTO);

	pthread_create(&hilo_escucha,NULL,(void*) esperar_cliente, &socket_escucha);

	//pthread_create(&recibir_localized,NULL,(void*)recibirLocalized,NULL);

	//printf("las listas son iguales ? = %d",igualdadDeListas(objetivos_globales,objetivos_globales));

	/* LIBERO ELEMENTOS */
	liberarArray(posicionesEntrenadores);
	liberarArray(pokesEntrenadores);
	liberarArray(pokesObjetivos);
	list_destroy_and_destroy_elements(pokemons_objetivos, free);
	list_destroy_and_destroy_elements(estado_new,liberarEntrenador);
	list_destroy_and_destroy_elements(objetivos_globales,liberarEspecie);

	/*CIERRO CONEXIONES*/
	close(socket_caught);
	close(socket_appeared);
	close(socket_localized);

	terminar_programa(); //Finalizo el programa

	return 0;
}

void* estado_exec(void* unEntrenador){
	t_entrenador* entrenador = (t_entrenador*) unEntrenador;
	if(entrenador->pokemonAMoverse != NULL){
		pthread_mutex_lock(&mutexEstadoExec);
		uint32_t distancia = distanciaEntrenadorPokemon(entrenador->posicionX,entrenador->posicionY,entrenador->pokemonAMoverse->posicionX,entrenador->pokemonAMoverse->posicionY);
		pthread_mutex_unlock(&mutexEstadoExec);
		ciclosConsumidos += distancia; //ACUMULO LOS CICLOS DE CPU CONSUMIDOS

		//CONEXION AL BROKER Y ENVIO DE MENSAJE CATCH
		int conexionExec = conectar_como_productor(config_get_string_value(config, "IP_BROKER"),config_get_string_value(config, "PUERTO_BROKER"));

		if(conexionExec != -1){
			entrenador->motivoBloqueo = MOTIVO_CATCH;
		}else{
			list_add(entrenador->pokesAtrapados,entrenador->pokemonAMoverse);
			entrenador->pokemonAMoverse = NULL;
			entrenador->motivoBloqueo = MOTIVO_CAUGHT;
		}

		catch_pokemon_msg* mensaje = catch_msg(entrenador->pokemonAMoverse->nombre,entrenador->pokemonAMoverse->posicionX,entrenador->pokemonAMoverse->posicionY);
		int idMensajeExec = enviar_mensaje(CATCH_POKEMON,mensaje,conexionExec); /* DONDE METO EL ID?*/
		close(conexionExec);

		//AGREGO A LA LISTA DE BLOQUEADOS
		entrenador->posicionX = entrenador->pokemonAMoverse->posicionX;
		entrenador->posicionY = entrenador->pokemonAMoverse->posicionY;
		entrenador->pokemonAMoverse = NULL;
		entrenador->idRecibido = idMensajeExec;

		return entrenador;
	}

	return NULL;
}

t_config* leer_config(){
	t_config* config_team = config_create(PATH_CONFIG);
	if(config_team == NULL){
		printf("Error abriendo el archivo de configuración\n");
		exit(-1);
	} else {
		printf("Archivo de configuracion leido.\n");
		return config_team;
	}
}

t_log* crear_log(){
	t_log* log_team = log_create(config_get_string_value(config,"LOG_FILE"), PROGRAM_NAME, 0, LOG_LEVEL_INFO);
	if(log_team == NULL){
		printf("Error creando el log\n");
		exit(-1);
	} else {
		printf("Log del proceso 'team' creado.\n\n");
		return log_team;
	}
}

void terminar_programa(){
	log_destroy(logger);
	config_destroy(config);
}

void inicializarPrograma(){
	//Leo el archivo de configuracion
	config = leer_config();
	//Inicializo el log
	logger = crear_log();
}
