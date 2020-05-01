#include "headers/team.h"
#include <conexiones.h>
#include <time.h>

int main(){

	inicializarPrograma(); //Inicializo logger y config

	char** posicionesEntrenadores = config_get_array_value(config,"POSICIONES_ENTRENADORES");
	char** pokesEntrenadores = config_get_array_value(config, "POKEMON_ENTRENADORES");
	char** pokesObjetivos = config_get_array_value(config, "OBJETIVOS_ENTRENADORES");

	t_list* entrenadores = list_create();

	crearListaDeEntrenadores(entrenadores,posicionesEntrenadores,pokesEntrenadores,pokesObjetivos);

	t_list* pokesObjetivoGlobal = crearListaPokesObjetivos(entrenadores);

	list_iterate(pokesObjetivoGlobal,mostrarString);

	uint32_t cantidad = cantidadDePokemons("Squirtle",pokesObjetivoGlobal);

	printf("\nLa cantidad son: %d\n\n",cantidad);

	t_list* listaObjetivos = crearListaObjetivoGlobal(pokesObjetivoGlobal);

	printf("La cantidad de pokemons sin repetir son: %d\n",list_size(listaObjetivos));
	printf("El pokemon de nombre %s",((t_especie*) list_get(listaObjetivos,1))->especie);
	printf(" aparece %d veces\n",((t_especie*)list_get(listaObjetivos,1))->cantidad);


	/* LIBERO ELEMENTOS */
	liberarArray(posicionesEntrenadores);
	liberarArray(pokesEntrenadores);
	liberarArray(pokesObjetivos);
	list_destroy_and_destroy_elements(pokesObjetivoGlobal, free);
	list_destroy_and_destroy_elements(entrenadores,liberarEntrenador);
	list_destroy_and_destroy_elements(listaObjetivos,liberarPokemon);



	char*  ipBroker = config_get_string_value(config,"IP_BROKER");
	char* puertoBroker = config_get_string_value(config,"PUERTO_BROKER");
	int  tiempoDeReconexion = config_get_int_value("TIEMPO_RECONEXION");
	while(1)
	{
		if(suscribirse_a_queue(APPEARED_POKEMON,ipBroker,puertoBroker) == -1)
		{
			printf("esperando Nueva Conexion");

		}
		sleep(tiempoDeReconexion);

	}

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

