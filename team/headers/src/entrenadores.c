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

t_list* crearListaDeEntrenadores(char** posicionesEntrenadores, char** pokesEntrenadores, char** pokesObjetivos)
{
	t_list* entrenadores = list_create();

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

	return entrenadores;
}

t_list* crearListaPokesObjetivos(t_list* entrenadores){

	t_list* pokesObjetivo = list_create();

	for(int i = 0; i < list_size(entrenadores); i++){

		t_entrenador* entrenador = (t_entrenador*) list_get(entrenadores, i);
		t_list* pokemons_entrenador = (t_list*) entrenador->pokesObjetivos;

		for(int j = 0; j < list_size(pokemons_entrenador); j++){
			char* pokemon = list_get(pokemons_entrenador, j);
			char* copia_pokemon = string_duplicate(pokemon);
			list_add(pokesObjetivo, copia_pokemon);

		}
	}

	return pokesObjetivo;
}

void mostrarEntrenador(void* entrenador)
{
	printf("\n\nLa identificacion del entrenador es: %d\n",((t_entrenador*)entrenador)->idEntrenador);
	printf("La posicion x del entrenador es: %d\n" ,((t_entrenador*)entrenador)->posicionX);
	printf("La posicion y del entrenador es: %d\n",((t_entrenador*)entrenador)->posicionY);
	printf("\nLos pokemon atrapados del entrenador son: \n");
	list_iterate((((t_entrenador*)entrenador)->pokesAtrapados),mostrarString);
	printf("\nLos pokemon obtenidos del entrenador son: \n");
	list_iterate((((t_entrenador*)entrenador)->pokesObjetivos),mostrarString);
	mostrarPokemon(((t_entrenador*)entrenador)->pokemonAMoverse);
}


t_entrenador* elEntrenadorMasCercanoAUnPokemonDeLaLista (t_list* listadoDeEntrenadores,t_list* listadoDePokemones)
{
	t_entrenador* entrenadorFlag = malloc(sizeof(t_entrenador));
	for(int i = 0; i < (list_size(listadoDeEntrenadores)-1);i++)
		{
			t_entrenador* pos1 = (t_entrenador*)list_get(listadoDeEntrenadores,i);
			t_entrenador* pos2 = (t_entrenador*)list_get(listadoDeEntrenadores,i+1);
			for(int j = 0; j < (list_size(listadoDePokemones)-1);j++)
			{
				t_pokemon* pokemonBase = (t_pokemon*)list_get(listadoDePokemones,j);
				t_entrenador* entrenadorMasCercano = elQueEstaMasCerca(pos1,pos2,pokemonBase);
				entrenadorFlag->posicionX = entrenadorMasCercano->posicionX;
				entrenadorFlag->posicionY = entrenadorMasCercano->posicionY;
				entrenadorFlag->pokesAtrapados = entrenadorMasCercano->pokesAtrapados;
				entrenadorFlag->pokesObjetivos = entrenadorMasCercano->pokesObjetivos;
				entrenadorFlag->idEntrenador = entrenadorMasCercano->idEntrenador;
				entrenadorFlag->pokemonAMoverse = pokemonBase;
			}
		}
	return entrenadorFlag;
}

t_entrenador* elQueEstaMasCerca(void* entrenador1, void* entrenador2,void* pokemon)
{
	uint32_t parcial1 = distanciaEntrenadorPokemon((((t_entrenador*)entrenador1)->posicionX),(((t_entrenador*)entrenador1)->posicionY),(((t_pokemon*)pokemon)->posicionX),((t_pokemon*)pokemon)->posicionY);
	uint32_t parcial2 = distanciaEntrenadorPokemon((((t_entrenador*)entrenador2)->posicionX),(((t_entrenador*)entrenador2)->posicionY),(((t_pokemon*)pokemon)->posicionX),((t_pokemon*)pokemon)->posicionY);
	if(parcial1 < parcial2)
	{
		return entrenador1;
	}
	else
	{
		return entrenador2;
	}
}

uint32_t distanciaEntrenadorPokemon(uint32_t posXEntrenador , uint32_t posYEntrenador, uint32_t posXPokemon, uint32_t PosYPokemon)
{
	uint32_t distanciaX = abs(posXEntrenador - posXPokemon);
	uint32_t distanciaY = abs(posYEntrenador - PosYPokemon);
	uint32_t distanciaTotal = distanciaX + distanciaY;
	return distanciaTotal;
}
