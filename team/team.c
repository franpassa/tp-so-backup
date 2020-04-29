#include "headers/team.h"

int main(){

	inicializarPrograma(); //Inicializo logger y config

	char** posicionesEntrenadores = config_get_array_value(config,"POSICIONES_ENTRENADORES");
	char** pokesEntrenadores = config_get_array_value(config, "POKEMON_ENTRENADORES");
	char** pokesObjetivos = config_get_array_value(config, "OBJETIVOS_ENTRENADORES");

	t_list* entrenadores = list_create();

	crearListaDeEntrenadores(entrenadores,posicionesEntrenadores,pokesEntrenadores,pokesObjetivos);

	t_list* pokesObjetivoGlobal = list_create();

	pokesObjetivoGlobal = crearListaPokesObjetivos(pokesObjetivoGlobal,entrenadores);

	list_iterate(pokesObjetivoGlobal,mostrarString);

	uint32_t cantidad = cantidadDePokemons("Squirtle",pokesObjetivoGlobal);

	printf("\nLa cantidad son: %d\n\n",cantidad);

	char* temp = malloc(sizeof(char));

	t_list* listaEvaluados = list_create();
	t_list* listaObjetivos = list_create();

	bool esDistinto(void* uno){
		return strcmp((char*)uno,temp)!=0;
	}

	for(int i=0;i<list_size(pokesObjetivoGlobal);i++){
		t_especie* especieTemporal = malloc(sizeof(t_especie));
		temp = list_get(pokesObjetivoGlobal,i);

		if(list_all_satisfy(listaEvaluados,esDistinto)){
			especieTemporal->especie = temp;
			especieTemporal->cantidad = cantidadDePokemons(temp,pokesObjetivoGlobal);
			list_add(listaObjetivos,especieTemporal);
			list_add(listaEvaluados,temp);
		}
	}

	printf("El pokemon de nombre %s",((t_especie*)list_get(listaObjetivos,0))->especie);
	printf(" aparece %d veces",((t_especie*)list_get(listaObjetivos,0))->cantidad);


	/* LIBERO ELEMENTOS */
	liberarArray(posicionesEntrenadores);
	liberarArray(pokesEntrenadores);
	liberarArray(pokesObjetivos);
	list_destroy_and_destroy_elements(entrenadores,liberarEntrenador);
	list_destroy(pokesObjetivoGlobal);

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

uint32_t cantidadDePokemons(char* especie, t_list* lista){
	uint32_t cantidad = 0;

	for(uint32_t i=0;i<list_size(lista);i++){
		if((strcmp(especie,(char*)list_get(lista,i)))==0){
			cantidad++;
		}
	}
	return cantidad;
}
