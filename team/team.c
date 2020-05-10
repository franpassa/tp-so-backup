#include "headers/team.h"

int main()
{
	inicializarPrograma(); //Inicializo logger y config

	char** posicionesEntrenadores = config_get_array_value(config,"POSICIONES_ENTRENADORES");
	char** pokesEntrenadores = config_get_array_value(config, "POKEMON_ENTRENADORES");
	char** pokesObjetivos = config_get_array_value(config, "OBJETIVOS_ENTRENADORES");

	estado_new = crearListaDeEntrenadores(posicionesEntrenadores,pokesEntrenadores,pokesObjetivos);

	t_list* pokemons_objetivos = crearListaPokesObjetivos(estado_new);

	t_list* objetivos_globales = crearListaObjetivoGlobal(pokemons_objetivos);

	/* LIBERO ELEMENTOS */
	liberarArray(posicionesEntrenadores);
	liberarArray(pokesEntrenadores);
	liberarArray(pokesObjetivos);
	list_destroy_and_destroy_elements(pokemons_objetivos, free);
	list_destroy_and_destroy_elements(estado_new,liberarEntrenador);
	list_destroy_and_destroy_elements(objetivos_globales,liberarEspecie);

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
		int conexionExec = crear_conexion(config_get_string_value(config, "IP_BROKER"),config_get_string_value(config, "PUERTO_BROKER"));
		catch_pokemon_msg* mensaje = catch_msg(entrenador->pokemonAMoverse->nombre,entrenador->pokemonAMoverse->posicionX,entrenador->pokemonAMoverse->posicionY);
		int idMensajeExec = enviar_mensaje(CATCH_POKEMON,mensaje,conexionExec); /* DONDE METO EL ID?*/
		close(conexionExec);

		//AGREGO A LA LISTA DE BLOQUEADOS
		entrenador->posicionX = entrenador->pokemonAMoverse->posicionX;
		entrenador->posicionY = entrenador->pokemonAMoverse->posicionY;
		entrenador->pokemonAMoverse = NULL;
		entrenador->idRecibido = idMensajeExec;
		entrenador->motivoBloqueo = MOTIVO_CATCH;

		pthread_exit(entrenador);
	}

	return NULL;
}

t_config* leer_config() {
	return config_create(PATH_CONFIG);
}

void terminar_programa(){
	log_destroy(logger);
	config_destroy(config);
}

void inicializarPrograma(){
	//Leo el archivo de configuracion
	config = config_create(PATH_CONFIG);
	printf("Archivo de configuracion leido.\n");

	//Inicializo el log
	logger = log_create(config_get_string_value(config,"LOG_FILE"), PROGRAM_NAME, 0, LOG_LEVEL_INFO);
	log_info(logger, "Log del proceso 'team' creado.");
	printf("Log del proceso 'team' creado.\n\n");
}
