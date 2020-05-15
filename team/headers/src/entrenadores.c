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
		entrenador->idRecibido = -1;
		entrenador->motivoBloqueo = NADA;
		entrenador->pokemonAMoverse = NULL;
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
	//mostrarPokemon(((t_entrenador*)entrenador)->pokemonAMoverse);

}

// iguala el segundo entrenador al primero
void igualarEntrenador(t_entrenador* unEntrenador, t_entrenador* otroEntrenador)
{
	unEntrenador->idEntrenador = otroEntrenador->idEntrenador;
	unEntrenador->idRecibido = otroEntrenador->idRecibido;
	unEntrenador->motivoBloqueo = otroEntrenador->motivoBloqueo;
	unEntrenador->pokemonAMoverse = otroEntrenador->pokemonAMoverse;
	unEntrenador->pokesAtrapados = otroEntrenador->pokesAtrapados;
	unEntrenador->pokesObjetivos = otroEntrenador->pokesObjetivos;
	unEntrenador->posicionX = otroEntrenador->posicionX;
	unEntrenador->posicionY = otroEntrenador->posicionY;
}

void setearEnCeroEntrenador (t_entrenador* unEntrenador)
{
	unEntrenador->idEntrenador = 0;
	unEntrenador->idRecibido = 0;
	unEntrenador->motivoBloqueo = NADA;
	unEntrenador->pokemonAMoverse = NULL;
	unEntrenador->pokesAtrapados = list_create();
	unEntrenador->pokesObjetivos = list_create();
	unEntrenador->posicionX = 0;
	unEntrenador->posicionY = 0;

}

// devuelve el pokemon de la lista que esta mas cerca a un entrenador
t_pokemon*  pokemonMasCercano (t_entrenador* unEntrenador, t_list* pokemons)
{
	t_pokemon* pokemonFlag =  malloc(sizeof(t_pokemon));
	setearEnCeroPokemon(pokemonFlag);
	igualarPokemons(pokemonFlag,list_get(pokemons,0));

	for(int i = 0; i < (list_size(pokemons)-1); i++)
	{
		t_pokemon* pokemonTemporal = list_get(pokemons,i+1);
		uint32_t distanciaA = distanciaEntrenadorPokemon(unEntrenador->posicionX,unEntrenador->posicionY,pokemonFlag->posicionX,pokemonFlag->posicionY);
		uint32_t distanciaB = distanciaEntrenadorPokemon(unEntrenador->posicionX,unEntrenador->posicionY,pokemonTemporal->posicionX,pokemonFlag->posicionY);
		if(distanciaA < distanciaB)
		{
			igualarPokemons(pokemonFlag,pokemonFlag);
		}
		else
		{
			igualarPokemons(pokemonFlag,pokemonTemporal);
		}
	}
	return pokemonFlag;

}

t_entrenador* entrenadorAReady(t_list* listaEntrenadores, t_list* listaPokemons)
{
	t_entrenador* entrenadorFlag = malloc(sizeof(t_entrenador));
	igualarEntrenador(entrenadorFlag,list_get(listaEntrenadores,0));
	if(list_size(listaEntrenadores) < 2){
		entrenadorFlag->pokemonAMoverse = pokemonMasCercano(entrenadorFlag,listaPokemons);
	}else{
		for(int i =  0; i < (list_size(listaEntrenadores)-1); i++){
			t_entrenador* entrenadorTemporal = list_get(listaEntrenadores,i+1);
			t_pokemon* pokemonParcialFlag = pokemonMasCercano(entrenadorFlag,listaPokemons);
			uint32_t distanciaFlag = distanciaEntrenadorPokemon(entrenadorFlag->posicionX,entrenadorFlag->posicionY,pokemonParcialFlag->posicionX,pokemonParcialFlag->posicionY);
			t_pokemon* pokemonEntrenadorTemporal= pokemonMasCercano(entrenadorTemporal,listaPokemons);
			uint32_t distanciaTemporal = distanciaEntrenadorPokemon(entrenadorTemporal->posicionX,entrenadorTemporal->posicionY,pokemonEntrenadorTemporal->posicionX,pokemonEntrenadorTemporal->posicionY);

			if(distanciaFlag > distanciaTemporal){
				igualarEntrenador(entrenadorFlag,entrenadorTemporal);
				entrenadorFlag->pokemonAMoverse = pokemonParcialFlag;
			}else{
				igualarEntrenador(entrenadorFlag,entrenadorFlag);
				entrenadorFlag->pokemonAMoverse = pokemonEntrenadorTemporal;
				}
		}
	}
	return entrenadorFlag;
}

bool bloqueadoPorNada(void* unEntrenador)
{
	return ((t_entrenador*)unEntrenador)->motivoBloqueo == MOTIVO_NADA;
}

t_list* todosLosEntrenadoresAPlanificar()
{
	t_list* temporal = list_create();
	list_add_all(temporal,estado_new);
	list_add_all(temporal,list_filter(estado_bloqueado,bloqueadoPorNada));

	return temporal;
}

bool estaEnLista(t_list* lista, t_entrenador* unEntrenador){
	for(int i = 0; i< list_size(lista); i++){
		if(unEntrenador == (t_entrenador*)list_get(lista,i)){
			return true;
		}
	}
	return false;
}

t_list* listaALaQuePertenece(t_entrenador* unEntrenador){
	if(estaEnLista(estado_new,unEntrenador)){
		return estado_new;
	}
	else if(estaEnLista(estado_bloqueado,unEntrenador)){
		return estado_bloqueado;
	}
	else{
		return estado_ready;
	}
}








