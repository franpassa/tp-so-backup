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
	estado_bloqueado = list_create();
	estado_ready = list_create();
	pokemons_objetivos = crearListaPokesObjetivos(estado_new);

	objetivos_globales = crearListaObjetivoGlobal(pokemons_objetivos); //t_especie*

	ids_enviados = list_create();

	enviar_gets(objetivos_globales);

//	int socket_escucha = iniciar_servidor(IP,PUERTO);
//
//	pthread_create(&hilo_escucha,NULL,(void*) esperar_cliente, &socket_escucha);
//	pthread_join(hilo_escucha, NULL);

	/* LIBERO ELEMENTOS */
	liberarArray(posicionesEntrenadores);
	liberarArray(pokesEntrenadores);
	liberarArray(pokesObjetivos);
	list_destroy_and_destroy_elements(pokemons_objetivos, free);
	list_destroy_and_destroy_elements(estado_new,liberarEntrenador);
	list_destroy_and_destroy_elements(objetivos_globales,liberarEspecie);

	/*CIERRO CONEXIONES*/
//	close(socket_caught);
//	close(socket_appeared);
//	close(socket_localized);

	terminar_programa(); //Finalizo el programa

	return 0;
}

void estado_exec(void* unEntrenador){

	bool esElMismo(void* entrenador){
		return entrenador == unEntrenador;
	}

	hayEntrenadorProcesando = true;
	t_entrenador* entrenador = list_remove_by_condition(listaALaQuePertenece(unEntrenador),esElMismo);
	char* ALGORITMO = config_get_string_value(config,"ALGORITMO_PLANIFICACION");
	int RETARDO = config_get_int_value(config,"RETARDO_CICLO_CPU");

	if(strcmp(ALGORITMO,"FIFO")==0){

		if(entrenador->pokemonAMoverse != NULL){

			uint32_t distancia = distanciaEntrenadorPokemon(entrenador->posicionX,entrenador->posicionY,entrenador->pokemonAMoverse->posicionX,entrenador->pokemonAMoverse->posicionY);
			uint32_t ciclos = 0;

			while(ciclos <= distancia){
				ciclos++;
				sleep(RETARDO);
			}

			pthread_mutex_lock(&mutexCiclosConsumidos);
			ciclosConsumidos += ciclos;//ACUMULO LOS CICLOS DE CPU CONSUMIDOS
			pthread_mutex_unlock(&mutexCiclosConsumidos);

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
			uint32_t* idMensajeExec = malloc(sizeof(int));
			*idMensajeExec = enviar_mensaje(CATCH_POKEMON,mensaje,conexionExec);
			list_add(ids_enviados,idMensajeExec);
			close(conexionExec);

			//AGREGO A LA LISTA DE BLOQUEADOS
			entrenador->posicionX = entrenador->pokemonAMoverse->posicionX;
			entrenador->posicionY = entrenador->pokemonAMoverse->posicionY;
			entrenador->pokemonAMoverse = NULL;
			entrenador->idRecibido = *idMensajeExec;
			list_add(estado_bloqueado,entrenador);
		}
	}
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

void mostrar_ids(void* id){
	printf("ID %d\n",*(int*)id);
}
