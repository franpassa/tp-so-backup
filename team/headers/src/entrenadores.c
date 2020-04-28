#include "../entrenadores.h"

uint32_t posicionXEntrenador(int nroEntrenador, char** posicionesEntrenadores){
	char* posTemp = posicionesEntrenadores[nroEntrenador];
	char** pos = string_split(posTemp,"|");
	uint32_t posX = atoi(pos[0]);
	liberarArray(pos);
	return posX;
}

uint32_t posicionYEntrenador(int nroEntrenador, char** posicionesEntrenadores){
	char* posTemp1 = posicionesEntrenadores[nroEntrenador];
	char** posTemp2 = string_split(posTemp1,"|");
	uint32_t posY = atoi(posTemp2[1]);
	liberarArray(posTemp2);
	return posY;
}

uint32_t cantidadTotalEntrenadores (char** posicionesEntrenadores){
	uint32_t i = 0;
	uint32_t acum = 0;
	while(posicionesEntrenadores[i] != NULL){
		acum += sizeof(posicionesEntrenadores[i]);
		i++;
	}
	return acum/sizeof(posicionesEntrenadores);
}

void liberarArray(char** array){
	for(uint32_t i=0;i<(cantidadTotalEntrenadores(array));i++){
		free(array[i]);
	}
	free(array);
}

t_list* insertarPokesEntrenador(uint32_t nroEntrenador, t_list* pokemons, char** pokesEntrenadores){
	void _a_la_lista(char* poke){
	  if (poke != NULL) {
	    list_add(pokemons, poke);
	  }
	}

	char** pokesEntrenador = string_split(pokesEntrenadores[nroEntrenador],"|");
	string_iterate_lines(pokesEntrenador,_a_la_lista);
	free(pokesEntrenador);

	return pokemons;
}

void liberarEntrenador(void* entrenador){
	list_destroy_and_destroy_elements(((t_entrenador*)entrenador)->pokesAtrapados,free);
	list_destroy_and_destroy_elements(((t_entrenador*)entrenador)->pokesObjetivos,free);
	free(entrenador);
}


void crearListaDeEntrenadores(t_list* entrenadores, char** posicionesEntrenadores, char** pokesEntrenadores, char** pokesObjetivos)
{
	for(uint32_t i=0;i<cantidadTotalEntrenadores(posicionesEntrenadores);i++)
	{
		t_entrenador* entrenador = malloc(sizeof(t_entrenador));
		entrenador->pokesAtrapados = list_create();
		entrenador->pokesObjetivos = list_create();

		entrenador->posicionX = posicionXEntrenador(i,posicionesEntrenadores);
		entrenador->posicionY = posicionYEntrenador(i,posicionesEntrenadores);
		entrenador->pokesAtrapados = insertarPokesEntrenador(i,entrenador->pokesAtrapados,pokesEntrenadores);
		entrenador->pokesObjetivos = insertarPokesEntrenador(i,entrenador->pokesObjetivos,pokesObjetivos);
		entrenador->idEntrenador = i;

		list_add(entrenadores,entrenador);
	}
}

t_list* crearListaPokesObjetivos(t_list* pokesObjetivos, t_list* entrenadores){
	for(int i = 0;i<list_size(entrenadores);i++){
		list_add_all(pokesObjetivos,((t_entrenador*)list_get(entrenadores,i))->pokesObjetivos);
	}
	return pokesObjetivos;
}
