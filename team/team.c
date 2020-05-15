#include "headers/team.h"

int main()
{
	inicializarPrograma(); //Inicializo logger y config

	inicializarVariables();

	//enviar_gets(objetivos_globales);

	int socket_escucha = iniciar_servidor(IP,PUERTO);
	if (socket_escucha == -1) abort(); //FINALIZA EL PROGRAMA EN CASO DE QUE FALLE LA INICIALIZACION DEL SERVIDOR

	pthread_create(&hilo_escucha,NULL,(void*) esperar_cliente, &socket_escucha);

	/* LIBERO ELEMENTOS */
	liberarArray(posicionesEntrenadores);
	liberarArray(pokesEntrenadores);
	liberarArray(pokesObjetivos);
	list_destroy_and_destroy_elements(pokemons_objetivos, free);
	list_destroy_and_destroy_elements(estado_new,liberarEntrenador);
	list_destroy_and_destroy_elements(objetivos_globales,liberarEspecie);

	pthread_join(hilo_escucha, NULL);

	/*CIERRO CONEXIONES*/
	close(socket_escucha);
//	close(socket_caught);
//	close(socket_appeared);
//	close(socket_localized);

	terminar_programa(); //Finalizo el programa

	return 0;
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

void inicializarVariables(){
	estado_bloqueado = list_create();
	estado_ready = list_create();
	estado_exit = list_create();
	ids_enviados = list_create();
	ids_recibidos = list_create();
	pokemons_recibidos = list_create();
	posicionesEntrenadores = config_get_array_value(config,"POSICIONES_ENTRENADORES");
	pokesEntrenadores = config_get_array_value(config, "POKEMON_ENTRENADORES");
	pokesObjetivos = config_get_array_value(config, "OBJETIVOS_ENTRENADORES");
	estado_new = crearListaDeEntrenadores(posicionesEntrenadores,pokesEntrenadores,pokesObjetivos);
	pokemons_objetivos = crearListaPokesObjetivos(estado_new);
	objetivos_globales = crearListaObjetivoGlobal(pokemons_objetivos); //t_especie*
}

void mostrar_ids(void* id){
	printf("ID %d\n",*(int*)id);
}
