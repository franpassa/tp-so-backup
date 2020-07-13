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

	char* a_agregar;

	void _a_la_lista(char* poke){
	  if (poke != NULL) {
	    list_add(pokemons, poke);
	  }
	}

	uint32_t tamanio = 0;
	for(tamanio=0; pokesEntrenadores[tamanio]!=NULL;tamanio++){}

	if(pokesEntrenadores[nroEntrenador] == NULL || string_is_empty(pokesEntrenadores[nroEntrenador])){
		a_agregar = string_new();
	} else {
		a_agregar = pokesEntrenadores[nroEntrenador];
	}



	char** pokes = string_split(a_agregar,"|");

	string_iterate_lines(pokes,_a_la_lista);
	free(pokes);

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

	uint32_t tamanio;
	for(tamanio=0; pokesEntrenadores[tamanio]!=NULL;tamanio++){}

	uint32_t i=0;
	for(i=0;i<cantidadTotalEntrenadores(posicionesEntrenadores);i++)
	{
		t_entrenador* entrenador = malloc(sizeof(t_entrenador));
		entrenador->pokesAtrapados = list_create();
		entrenador->pokesObjetivos = list_create();
		entrenador->posicionX = posicionXEntrenador(i,posicionesEntrenadores);
		entrenador->posicionY = posicionYEntrenador(i,posicionesEntrenadores);
		if(i<=tamanio){entrenador->pokesAtrapados = insertarPokesEntrenador(i,entrenador->pokesAtrapados,pokesEntrenadores);}
		entrenador->pokesObjetivos = insertarPokesEntrenador(i,entrenador->pokesObjetivos,pokesObjetivos);
		entrenador->idEntrenador = i;
		entrenador->idRecibido = 0;
		entrenador->motivoBloqueo = MOTIVO_NADA;
		entrenador->pokemonAMoverse = NULL;
		entrenador->estimacion = ESTIMACION_INICIAL;
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
	printf("\nLa identificacion del entrenador es: %d\n",((t_entrenador*)entrenador)->idEntrenador);
	printf("La posicion x del entrenador es: %d\n" ,((t_entrenador*)entrenador)->posicionX);
	printf("La posicion y del entrenador es: %d\n",((t_entrenador*)entrenador)->posicionY);
	printf("Los pokemon atrapados del entrenador son: \n");
	list_iterate((((t_entrenador*)entrenador)->pokesAtrapados),mostrarString);
	printf("Los pokemon objetivo del entrenador son: \n");
	list_iterate((((t_entrenador*)entrenador)->pokesObjetivos),mostrarString);
	printf("El motivo de bloqueo del entrenador es: %d\n",((t_entrenador*) entrenador)->motivoBloqueo);
	if(((t_entrenador*)entrenador)->pokemonAMoverse != NULL){mostrarPokemon(((t_entrenador*)entrenador)->pokemonAMoverse);}

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
	unEntrenador->estimacion = otroEntrenador->estimacion;
}

void setearEnCeroEntrenador (t_entrenador* unEntrenador)
{
	unEntrenador->idEntrenador = 0;
	unEntrenador->idRecibido = 0;
	unEntrenador->motivoBloqueo = MOTIVO_NADA;
	unEntrenador->pokemonAMoverse = NULL;
	unEntrenador->pokesAtrapados = list_create();
	unEntrenador->pokesObjetivos = list_create();
	unEntrenador->posicionX = 0;
	unEntrenador->posicionY = 0;

}

// devuelve el pokemon de la lista que esta mas cerca a un entrenador
t_pokemon*  pokemonMasCercano (t_entrenador* unEntrenador, t_list* pokemons)
{
	t_pokemon* pokemonFlag = malloc(sizeof(t_pokemon));
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
		for(int i =  0; i < (list_size(listaEntrenadores)-1); i++)
		{
			t_entrenador* entrenadorTemporal = list_get(listaEntrenadores,i+1);
			t_pokemon* pokemonParcialFlag = pokemonMasCercano(entrenadorFlag,listaPokemons);
			uint32_t distanciaFlag = distanciaEntrenadorPokemon(entrenadorFlag->posicionX,entrenadorFlag->posicionY,pokemonParcialFlag->posicionX,pokemonParcialFlag->posicionY);
			t_pokemon* pokemonEntrenadorTemporal= pokemonMasCercano(entrenadorTemporal,listaPokemons);
			uint32_t distanciaTemporal = distanciaEntrenadorPokemon(entrenadorTemporal->posicionX,entrenadorTemporal->posicionY,pokemonEntrenadorTemporal->posicionX,pokemonEntrenadorTemporal->posicionY);

			if(distanciaFlag > distanciaTemporal)
			{
				igualarEntrenador(entrenadorFlag,entrenadorTemporal);
				entrenadorFlag->pokemonAMoverse = pokemonParcialFlag;
			}
			else
			{
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

bool bloqueadoPorDeadlock(t_entrenador* unEntrenador)
{
	return unEntrenador->motivoBloqueo == ESPERA_DEADLOCK;
}

t_list* todosLosEntrenadoresAPlanificar()
{
	t_list* temporal = list_filter(estado_bloqueado,bloqueadoPorNada);
	list_add_all(temporal,estado_new);

	return temporal;
}

bool estaEnLista(t_list* lista, t_entrenador* entrenador){

	bool esElMismo(t_entrenador* unEntrenador){
		return unEntrenador->idEntrenador == entrenador->idEntrenador;
	}

	return list_any_satisfy(lista,(void*) esElMismo);
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

bool puedeAtrapar(t_entrenador* entrenador){
	return list_size(entrenador->pokesObjetivos) > list_size(entrenador->pokesAtrapados);
}

// funcion por si las dudas
void moverEntrenadorX(t_entrenador* unEntrenador, uint32_t posX)
{
    while(abs(unEntrenador->posicionX - posX) > 0)
    {
        sleep(retardoCpu);
        if(unEntrenador->posicionX > posX)
        {
            unEntrenador->posicionX --;
        }
        else
        {
            unEntrenador->posicionX ++;
        }
        pthread_mutex_lock(&mutexLogEntrenador);
        log_info(logger,"El entrenador %d se movio a la posicion (%d,%d).",unEntrenador->idEntrenador, unEntrenador->posicionX, unEntrenador->posicionY);
        printf("el entrenador %d se movio a la posicion (%d,%d)\n",unEntrenador->idEntrenador, unEntrenador->posicionX, unEntrenador->posicionY);
        pthread_mutex_unlock(&mutexLogEntrenador);
    }
}

void moverEntrenadorY(t_entrenador* unEntrenador, uint32_t posY)
{
    while(abs(unEntrenador->posicionY - posY) > 0)
    {
        sleep(retardoCpu);
        if(unEntrenador->posicionY > posY)
        {
            unEntrenador->posicionY --;
        }
        else
        {
            unEntrenador->posicionY ++;
        }
        pthread_mutex_lock(&mutexLogEntrenador);
        log_info(logger,"El entrenador %d se movio a la posicion (%d,%d).",unEntrenador->idEntrenador, unEntrenador->posicionX, unEntrenador->posicionY);
        printf("el entrenador %d se movio a la posicion (%d,%d)\n",unEntrenador->idEntrenador, unEntrenador->posicionX, unEntrenador->posicionY);
        pthread_mutex_unlock(&mutexLogEntrenador);
    }
}

void moverSinDesalojar(t_entrenador* unEntrenador, uint32_t posX, uint32_t posY)
{
    moverEntrenadorX(unEntrenador,posX);
    moverEntrenadorY(unEntrenador,posY);
}

void moverConDesalojoPorRR(t_entrenador* unEntrenador, uint32_t posX, uint32_t posY){

	uint32_t distanciaRecorrida = 0;

	uint32_t distancia = distanciaEntrenadorPokemon(unEntrenador->posicionX, unEntrenador->posicionY,posX,posY);

	while(distanciaRecorrida < QUANTUM && distancia!=0){

		if(abs(unEntrenador->posicionX - posX) > 0){
		        sleep(retardoCpu);
		        if(unEntrenador->posicionX > posX){
		            unEntrenador->posicionX --;
		        } else {
		            unEntrenador->posicionX ++;
		        }
		        printf("el entrenador %d se movio a la posicion (%d,%d)\n",unEntrenador->idEntrenador, unEntrenador->posicionX, unEntrenador->posicionY);
		        distanciaRecorrida++;
		        distancia = distanciaEntrenadorPokemon(unEntrenador->posicionX, unEntrenador->posicionY,posX,posY);
		        if(distanciaRecorrida == QUANTUM){break;}
		    }

		if(abs(unEntrenador->posicionY - posY) > 0){
		        sleep(retardoCpu);
		        if(unEntrenador->posicionY > posY){
		            unEntrenador->posicionY --;
		        } else {
		            unEntrenador->posicionY ++;
		        }
		        printf("el entrenador %d se movio a la posicion (%d,%d)\n",unEntrenador->idEntrenador, unEntrenador->posicionX, unEntrenador->posicionY);
		        distanciaRecorrida++;
		        distancia = distanciaEntrenadorPokemon(unEntrenador->posicionX, unEntrenador->posicionY,posX,posY);
		        if(distanciaRecorrida == QUANTUM){break;}
		    }
	}
}

void moverEntrenador(t_entrenador* unEntrenador, uint32_t posX, uint32_t posY)
{
	if(string_equals_ignore_case(ALGORITMO,"FIFO") || string_equals_ignore_case(ALGORITMO,"SJF-SD"))
	{
		moverSinDesalojar(unEntrenador,posX,posY);
	}

	if(string_equals_ignore_case(ALGORITMO,"RR")){

		moverConDesalojoPorRR(unEntrenador,posX,posY);
	}
}

t_list* pokemonesAlPedo(t_entrenador* unEntrenador)
{
	t_list* losQueSobranDistintos = pokemonsQueLeSobranDistintos(unEntrenador);

	t_list* losQueSobranDeEspeciesCapturadas = pokemonsRepetidos(unEntrenador);

	list_add_all(losQueSobranDistintos,losQueSobranDeEspeciesCapturadas);

	list_destroy(losQueSobranDeEspeciesCapturadas);

	return losQueSobranDistintos;
}

t_list* pokemonsRepetidos(t_entrenador* unEntrenador){

	t_list* resultado = pokemonsQueLogroAtrapar(unEntrenador);

	t_list* a_devolver = list_create();

	void agregarSiCorresponde(char* elemento){

		uint32_t cantidad = cantidadAtrapadosDeMas(unEntrenador,elemento);

		if(cantidad>0){
			for(uint32_t i = 0; i<cantidad;i++){
				char* a_agregar = string_duplicate(elemento);
				list_add(a_devolver,a_agregar);
			}
		}
	}

	list_iterate(resultado,(void*) agregarSiCorresponde);

	list_destroy(resultado);

	return a_devolver;

}

uint32_t cantidadAtrapadosDeMas(t_entrenador* entrenador,char* unaEspecie){

	bool esIgual(char* otroPokemon)
	{
		return string_equals_ignore_case(otroPokemon,unaEspecie);
	}

	uint32_t atrapados = list_count_satisfying(entrenador->pokesAtrapados,(void*) esIgual);
	uint32_t objetivo = list_count_satisfying(entrenador->pokesObjetivos,(void*) esIgual);

	return atrapados-objetivo;
}

t_list* pokemonsQueLogroAtrapar(t_entrenador* unEntrenador){

	bool estaEnLista(char* pokemon)
	{
		bool esIgual(char* otroPokemon)
		{
			return string_equals_ignore_case(otroPokemon,pokemon);
		}
		return list_any_satisfy(unEntrenador->pokesObjetivos,(void*)esIgual);
	}

	return list_filter(unEntrenador->pokesAtrapados,(void*) estaEnLista);
}

t_list* pokemonesQueLeFaltan(t_entrenador* unEntrenador)
{
	bool noEstaEnLista(char* pokemon)
	{
		bool esIgual(char* otroPokemon)
		{
			return string_equals_ignore_case(otroPokemon,pokemon);
		}
		return !list_all_satisfy(unEntrenador->pokesAtrapados,(void*)esIgual);
	}
	return list_filter(unEntrenador->pokesObjetivos,(void*)noEstaEnLista);
}

t_list* pokemonsQueLeSobranDistintos(t_entrenador* unEntrenador){

	bool noEstaEnLista(char* pokemon)
	{
		bool esIgual(char* otroPokemon)
		{
			return string_equals_ignore_case(otroPokemon,pokemon);
		}
		return !list_any_satisfy(unEntrenador->pokesObjetivos,(void*)esIgual);
	}

	return list_filter(unEntrenador->pokesAtrapados,(void*)noEstaEnLista);
}

// devuelve una lista de los entrenadores que tienen el/los pokemon que me faltan
// el t_list* lista es una lista con todos los blockeados menos el mismo

t_list* quienesTienenElPokeQueMeFalta(t_entrenador* unEntrenador, t_list* lista)
{
	t_list* pokemons_al_pedo;
	t_list* pokemonsQueFaltan;

	// nos dice si el segundo entrenador tiene algun pokemon de sobra que necesite el primero
	bool tieneUnPokemonQueLeFalta( t_entrenador* otroEntrenador)
	{
		bool estaEnLista(char* pokemon)
		{
			bool esIgual(char* otroPokemon)
			{
				return string_equals_ignore_case(otroPokemon,pokemon);
			}

			pokemons_al_pedo = pokemonesAlPedo(otroEntrenador);

			return list_any_satisfy(pokemons_al_pedo,(void*)esIgual);
		}

		pokemonsQueFaltan = pokemonesQueLeFaltan(unEntrenador);

		return list_any_satisfy(pokemonsQueFaltan,(void*)estaEnLista);
	}

	t_list* resultado = list_filter(lista,(void*)tieneUnPokemonQueLeFalta);

//	list_destroy(pokemons_al_pedo);
//	list_destroy(pokemonsQueFaltan);

	return resultado;
}

uint32_t retornarIndice(t_list* lista, char* nombre)
{
	for(uint32_t i = 0; i<list_size(lista);i++)
	{

		if(string_equals_ignore_case(nombre,list_get(lista,i)))
		{
			return i;
		}

	}
	return -1;
}

void realizarCambio(t_entrenador* entrenador1, t_entrenador* entrenador2)
{
	log_info(logger,"INTERCAMBIO de pokemons entre los entrenadores %d y %d.",entrenador1->idEntrenador, entrenador2->idEntrenador);
	char* pokemon(t_list* lista1)
	{
		bool estaEnLista(char* pokemon)
		{
			bool esIgual(char* otroPokemon)
			{
				return string_equals_ignore_case(otroPokemon,pokemon);
			}
			return list_any_satisfy(lista1,(void*)esIgual);
		}
		return list_find(entrenador2->pokesAtrapados,(void*)estaEnLista);
	}

	t_list* pokemones = pokemonesAlPedo(entrenador1);
	uint32_t a = retornarIndice(entrenador1->pokesAtrapados,list_get(pokemones,0));
	uint32_t indiceDelPokemonDondeEstaEnElEntrenador2 = retornarIndice(entrenador2->pokesAtrapados,pokemon(entrenador1->pokesObjetivos));

	char* flag = list_remove(entrenador2->pokesAtrapados,indiceDelPokemonDondeEstaEnElEntrenador2);
	char* flag2 = list_replace(entrenador1->pokesAtrapados,a,flag);
	sleep(5);
	list_add(entrenador2->pokesAtrapados,flag2);

	list_destroy(pokemones);
}

t_list* crearListaObjetivosPosta(t_list* pokesObjetivosGlobal, t_list* entrenadoresNew)
{
	t_list* lista = list_duplicate(pokesObjetivosGlobal);

	void actualizarEstado(t_entrenador* unEntrenador)
	{
		for(int i = 0; i < list_size(unEntrenador->pokesAtrapados); i++)
		{
			bool algunaEsLaMismaEspecie(char* unaEspecie)
			{
				bool esLaMismaEspecie(t_especie* otraEspecie)
				{
					return string_equals_ignore_case(unaEspecie,otraEspecie->especie);
				}
				return list_any_satisfy(lista,(void*) esLaMismaEspecie);
			}

			if(algunaEsLaMismaEspecie(list_get(unEntrenador->pokesAtrapados,i)))
			{
				sacar1(list_get(unEntrenador->pokesAtrapados,i),lista);
			}
		}
	}
	list_iterate(entrenadoresNew,(void*) actualizarEstado);
	return lista;
}

void sacar1(char* nombre, t_list* listaDeEspecies)
{
	bool comparar(t_especie* unPokemon)
	{
		return string_equals_ignore_case(unPokemon->especie,nombre);
	}

	if(estaEnListaEspecie(nombre, listaDeEspecies))
	{
		t_especie* a =list_find(listaDeEspecies,(void*) comparar);
		a->cantidad --;
	}
}

void recalcularEstimacion(t_entrenador* entrenador,uint32_t ciclosRecorridos){
	entrenador->estimacion = ALPHA*ciclosRecorridos + (1-ALPHA)*entrenador->estimacion;
}

t_entrenador* elDeMenorEstimacion(t_list* entrenadores){ //DEVUELVE EL ENTRENADOR DE MENOR ESTIMACION Y LO BORRA DE LA LISTA

	t_entrenador* entrenadorInicial = (t_entrenador*) list_get(entrenadores,0);
	double estimacionFlag = entrenadorInicial->estimacion;

	void buscarMenor(t_entrenador* unEntrenador){
		double estimacion = unEntrenador->estimacion;
		if(estimacion < estimacionFlag){
			estimacionFlag = unEntrenador->estimacion;
		}
	}

	list_iterate(entrenadores,(void*) buscarMenor);

	bool tieneLaMenor(t_entrenador* unEntrenador){
		return fabs(unEntrenador->estimacion)== fabs(estimacionFlag);
	}

	return list_remove_by_condition(entrenadores,(void*)tieneLaMenor);
}








