#include "../pokemon.h"


uint32_t cantidadDePokemonsPorEspecie(char* especie, t_list* lista){
	uint32_t cantidad = 0;

	for(uint32_t i=0;i<list_size(lista);i++){
		if((strcmp(especie,(char*)list_get(lista,i)))==0){
			cantidad++;
		}
	}
	return cantidad;
}

t_list* crearListaObjetivoGlobal(t_list* pokesObjetivoGlobal){

	t_list* objetivoGlobal = list_create();

	for(int i=0; i < list_size(pokesObjetivoGlobal); i++){

		char* pokemon = (char*) list_get(pokesObjetivoGlobal, i);

		bool es_el_mismo(t_especie* especie){
			return string_equals_ignore_case(pokemon, especie->especie);
		}

		t_especie* pokemon_ya_en_lista = list_find(objetivoGlobal, (void*) es_el_mismo);

		if(pokemon_ya_en_lista == NULL){

			char* copia_pokemon = string_duplicate(pokemon);
			t_especie* nueva_especie = malloc(sizeof(t_especie));
			nueva_especie->especie = copia_pokemon;
			nueva_especie->cantidad = 1;
			list_add(objetivoGlobal,nueva_especie);
		} else {
			pokemon_ya_en_lista->cantidad++;
		}

	}

	return objetivoGlobal;
}

void agregarLocalizedRecibidoALista(t_list* pokemonsRecibidos, localized_pokemon_msg* pokemons){
	int j = 1;
	for(int i=0; i<(pokemons->cantidad_posiciones)*2; i+=2){
			t_pokemon* pokemon = malloc(sizeof(t_pokemon));
			char* copia_nombre = string_duplicate(pokemons->nombre_pokemon);
			pokemon->nombre = copia_nombre;
			pokemon->posicionX = pokemons->pares_coordenadas[i];
			pokemon->posicionY = pokemons->pares_coordenadas[j];
			list_add(pokemonsRecibidos,pokemon);
			j+=2;
	}
}

void agregarAppearedRecibidoALista(t_list* pokemonsRecibidos, appeared_pokemon_msg* pokemon){
	t_pokemon* a_agregar = malloc(sizeof(t_pokemon));
	char* copia_nombre = string_duplicate(pokemon->nombre_pokemon);
	a_agregar->nombre = copia_nombre;
	a_agregar->posicionX = pokemon->coordenada_X;
	a_agregar->posicionY = pokemon->coordenada_Y;
	list_add(pokemonsRecibidos,a_agregar);
}

void mostrarString(void *elemento){
  printf("%s\n", (char *)elemento);
}

void mostrarPokemon(void* pokemon){
    printf("El nombre del pokemon es: ");
    mostrarString((((t_pokemon*)pokemon)->nombre));
    printf("La pos en X es: %d\n",((t_pokemon*)pokemon)->posicionX);
    printf("La pos en y es: %d\n",((t_pokemon*)pokemon)->posicionY);
}

void mostrarEspecie(void* especie)
{
	mostrarString(((t_especie*)especie)->especie);
	printf(" cantidad = %d \n",((t_especie*)especie)->cantidad);
}

void liberarEspecie(void* especie){
	t_especie* liberar = (t_especie*)especie;
	free(liberar->especie);
	free(liberar);
}

void liberarPokemon(void* pokemon){
	t_pokemon* liberar = (t_pokemon*)pokemon;
	free(liberar->nombre);
	free(liberar);
}

void igualarPokemons(t_pokemon* unPokemon, t_pokemon* otroPokemon) // iguala el primer pokemon al segundo
{
	unPokemon->nombre = otroPokemon->nombre;
	unPokemon->posicionX = otroPokemon->posicionX;
	unPokemon->posicionY = otroPokemon->posicionY;
}

void setearEnCeroPokemon(t_pokemon* unPokemon)
{
	unPokemon->nombre = NULL;
	unPokemon->posicionX = 0;
	unPokemon->posicionY = 0;
}


// devuelve la distancia entre un entrenador y un pokemon
uint32_t distanciaEntrenadorPokemon(uint32_t posXEntrenador , uint32_t posYEntrenador, uint32_t posXPokemon, uint32_t posYPokemon)
{
	uint32_t distanciaX = abs(posXEntrenador - posXPokemon);
	uint32_t distanciaY = abs(posYEntrenador - posYPokemon);
	uint32_t distanciaTotal = distanciaX + distanciaY;
	return distanciaTotal;
}

bool sonIguales(t_list* pokemonsAtrapados,t_list* pokemonsObjetivos)
{
	if(list_size(pokemonsObjetivos) != list_size(pokemonsAtrapados))
	{
		return false;
	}
	else
	{
		return todosLosElementosDeLaPrimerListaEstanEnLaSegunda(pokemonsAtrapados,pokemonsObjetivos);
	}
}

bool todosLosElementosDeLaPrimerListaEstanEnLaSegunda(t_list* listaA,t_list* listaB)
{
	bool comparar(char* unPokemon){
		return perteneceALaSegundaLista(unPokemon,listaB);
	}

	return list_all_satisfy(listaA,(void*) comparar);
}

bool perteneceALaSegundaLista(char* pokemon,t_list* listaDePokemons) // esta compara el nombre del pokemon con una lista de strings
{
	bool comparar(char* unPokemon){
		return string_equals_ignore_case(unPokemon,pokemon);
	}

	return list_any_satisfy(listaDePokemons,(void*) comparar);
}

bool estaEnLaLista(char* unNombre, t_list* listadoDePokemons) // esta compara el nombre del pokemon con una lista de pokemons
{
	bool comparar(t_pokemon* pokemon){
		return string_equals_ignore_case(pokemon->nombre, unNombre);
	}

	return list_any_satisfy(listadoDePokemons,(void*) comparar);
}

bool estaEnListaPokemon(t_list* lista, t_pokemon* pokemon){
	for(int i = 0; i< list_size(lista); i++){
		if(string_equals_ignore_case(pokemon->nombre,((t_pokemon*)list_get(lista,i))->nombre)){
			return true;
		}
	}
	return false;
}
