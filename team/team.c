#include "headers/team.h"

void crearHilos(t_list* entrenadores)
{
	pthread_t  hiloEntrenador[list_size(entrenadores)];
	for(int i = 0; i<list_size(entrenadores); i++)
	{
		sleep(5);
		t_entrenador* entrenador_aux = (t_entrenador*)list_get(entrenadores,i);
		pthread_create(&hiloEntrenador[i],NULL,(void*) mostrarEntrenador, entrenador_aux);
		pthread_join(hiloEntrenador[i],NULL);
	}
}

int main(){

	inicializarPrograma(); //Inicializo logger y config

	char** posicionesEntrenadores = config_get_array_value(config,"POSICIONES_ENTRENADORES");
	char** pokesEntrenadores = config_get_array_value(config, "POKEMON_ENTRENADORES");
	char** pokesObjetivos = config_get_array_value(config, "OBJETIVOS_ENTRENADORES");

	t_list* entrenadores = crearListaDeEntrenadores(posicionesEntrenadores,pokesEntrenadores,pokesObjetivos);

	t_list* pokesObjetivoGlobal = crearListaPokesObjetivos(entrenadores);

	list_iterate(pokesObjetivoGlobal,mostrarString);

	uint32_t cantidad = cantidadDePokemons("Squirtle",pokesObjetivoGlobal);

	printf("\nLa cantidad son: %d\n\n",cantidad);

	t_list* listaObjetivos = crearListaObjetivoGlobal(pokesObjetivoGlobal);

	printf("La cantidad de pokemons sin repetir son: %d\n",list_size(listaObjetivos));
	printf("El pokemon de nombre %s",((t_especie*) list_get(listaObjetivos,1))->especie);
	printf(" aparece %d veces\n",((t_especie*)list_get(listaObjetivos,1))->cantidad);

	crearHilos(entrenadores);

	//Prototipo del envio de mensajes GET al Broker (No va a funcionar sin conectarse al broker y obtener el socket)

	/*for(int i=0; i<list_size(listaObjetivos); i++){
		t_especie* pokemon = (t_especie*) list_get(listaObjetivos,i);
		char* nombrePokemon = string_duplicate(pokemon->especie);
		get_pokemon_msg* mensaje = get_msg(nombrePokemon);
		enviar_mensaje(GET_POKEMON,mensaje,1); //Aca supongo que la conexion con el Broker me devuelve un 1.
		free(mensaje->nombre_pokemon);
		free(mensaje);
	}*/


	/* LIBERO ELEMENTOS */
	liberarArray(posicionesEntrenadores);
	liberarArray(pokesEntrenadores);
	liberarArray(pokesObjetivos);
	list_destroy_and_destroy_elements(pokesObjetivoGlobal, free);
	list_destroy_and_destroy_elements(entrenadores,liberarEntrenador);
	list_destroy_and_destroy_elements(listaObjetivos,liberarPokemon);

	// conectar el team al boker
//	char*  ipBroker = config_get_string_value(config,"IP_BROKER");
//	char* puertoBroker = config_get_string_value(config,"PUERTO_BROKER");
//	int  tiempoDeReconexion = config_get_int_value(config,"TIEMPO_RECONEXION");


	/*while(1)
	{
		if(suscribirse_a_queue(APPEARED_POKEMON,ipBroker,puertoBroker) == -1)
		{
			printf("Esperando Nueva Conexion /n");

		}
		sleep(tiempoDeReconexion);

	}*/

	terminar_programa(); //Finalizo el programa

	return 0;
}

t_config* leer_config() {
	return config_create(PATH_CONFIG);
}

void terminar_programa(){
	log_destroy(logger);
	config_destroy(config);
}

void mostrarString(void *elemento){
  printf("%s\n", (char *)elemento);
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

