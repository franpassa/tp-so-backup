#include "headers/team.h"

int main()
{
	inicializarPrograma(); //Inicializo logger y config
	inicializarVariables();

	printf("\n");
	int socket_escucha = iniciar_servidor(IP,PUERTO);
	printf("\n");
	if (socket_escucha == -1) abort(); //FINALIZA EL PROGRAMA EN CASO DE QUE FALLE LA INICIALIZACION DEL SERVIDOR

	enviar_gets(objetivos_globales); // ENVIO MENSAJES GET_POKEMON AL BROKER.

	pthread_create(&hilo_pasar_a_ready,NULL,(void*) pasar_a_ready, NULL);

	pthread_create(&hilo_escucha,NULL,(void*) esperar_cliente, &socket_escucha);

	pthread_create(&hilo_estado_exec, NULL, (void*) estado_exec, NULL);

	pthread_create(&hilo_recibir_localized, NULL, (void*) recibirLocalized, NULL);

	pthread_create(&hilo_recibir_caught, NULL, (void*) recibirCaught, NULL);

	pthread_create(&hilo_recibir_appeared, NULL, (void*) recibirAppeared, NULL);

	pthread_create(&hilo_deadlock,NULL,(void*)deadlock,NULL);

	pthread_join(hilo_escucha, NULL);
	pthread_join(hilo_estado_exec, NULL);
	pthread_join(hilo_pasar_a_ready, NULL);
	pthread_join(hilo_recibir_localized, NULL);
	pthread_join(hilo_recibir_appeared, NULL);
	pthread_join(hilo_recibir_caught, NULL);
	pthread_join(hilo_deadlock,NULL);

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
	liberarVariables();
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
	pokemons_recibidos = list_create();
	pokemons_recibidos_historicos = list_create();
	pthread_mutex_init(&mutexReconexion, NULL);
	pthread_mutex_init(&mutexCiclosConsumidos, NULL);
	pthread_mutex_init(&mutexEstadoBloqueado, NULL);
	pthread_mutex_init(&mutexEstadoNew, NULL);
	pthread_mutex_init(&mutexEstadoReady, NULL);
	pthread_mutex_init(&mutexIdsEnviados, NULL);
	pthread_mutex_init(&mutexPokemonsRecibidos, NULL);
	pthread_mutex_init(&mutexPokemonsRecibidosHistoricos, NULL);
	pthread_mutex_init(&mutexLog, NULL);
	pthread_mutex_init(&mutexLogEntrenador, NULL);
	pthread_mutex_init(&mutexHayEntrenadorProcesando, NULL);
	sem_init(&semCaught, 0, 1);
	sem_init(&semLocalized, 0, 1);
	sem_init(&semAppeared, 0, 1);

	posicionesEntrenadores = config_get_array_value(config,"POSICIONES_ENTRENADORES");
	pokesEntrenadores = config_get_array_value(config, "POKEMON_ENTRENADORES");
	pokesObjetivos = config_get_array_value(config, "OBJETIVOS_ENTRENADORES");
	ip_broker = config_get_string_value(config, "IP_BROKER");
	puerto_broker = config_get_string_value(config, "PUERTO_BROKER");
	ALGORITMO = config_get_string_value(config,"ALGORITMO_PLANIFICACION");
	retardoCpu = config_get_int_value(config, "RETARDO_CICLO_CPU");
	tiempo_reconexion = config_get_int_value(config, "TIEMPO_RECONEXION");
	IP = config_get_string_value(config, "IP_TEAM");
	PUERTO = config_get_string_value(config, "PUERTO_TEAM");

	estado_new = crearListaDeEntrenadores(posicionesEntrenadores,pokesEntrenadores,pokesObjetivos);
	pokemons_objetivos = crearListaPokesObjetivos(estado_new);
	objetivos_globales = crearListaObjetivoGlobal(pokemons_objetivos);
	hayEntrenadorProcesando = false;
}

void liberarVariables()
{
	liberarArray(posicionesEntrenadores);
	liberarArray(pokesEntrenadores);
	liberarArray(pokesObjetivos);
	list_destroy_and_destroy_elements(pokemons_objetivos, free);
	list_destroy_and_destroy_elements(estado_new,liberarEntrenador);
	list_destroy_and_destroy_elements(objetivos_globales,liberarEspecie);
	list_destroy(estado_bloqueado);
	list_destroy(estado_ready);
	list_destroy(estado_exit);
	list_destroy(ids_enviados);
	list_destroy(pokemons_recibidos);
	list_destroy(pokemons_recibidos_historicos);
}

void mostrar_ids(void* id){
	printf("ID %d\n",*(int*)id);
}
