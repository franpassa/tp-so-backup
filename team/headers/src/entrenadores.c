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
	t_entrenador* unEntrenador = (t_entrenador*) entrenador;
	list_destroy_and_destroy_elements(((t_entrenador*)entrenador)->pokesAtrapados,free);
	//list_destroy_and_destroy_elements(((t_entrenador*)entrenador)->pokesObjetivos,free);
	free(unEntrenador);
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
		entrenador->ciclosAcumulados = 0;
		entrenador->posXAMoverse = 0;
		entrenador->posYAMoverse = 0;
		entrenador->yaPasoAReadyUnaVez = false;
		list_add(entrenadores,entrenador);
		sem_post(&semEntrenadoresAPlanificar);
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
	printf("Los ciclos acumulados del entrenador son: %d\n",((t_entrenador*) entrenador)->ciclosAcumulados);
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
	unEntrenador->ciclosAcumulados = otroEntrenador->ciclosAcumulados;
	unEntrenador->posXAMoverse = otroEntrenador->posXAMoverse;
	unEntrenador->posYAMoverse = otroEntrenador->posYAMoverse;
	unEntrenador->yaPasoAReadyUnaVez = otroEntrenador->yaPasoAReadyUnaVez;
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
	unEntrenador->ciclosAcumulados = 0;
	unEntrenador->posXAMoverse = 0;
	unEntrenador->posYAMoverse = 0;
	unEntrenador->yaPasoAReadyUnaVez = false;
}

// devuelve el pokemon de la lista que esta mas cerca a un entrenador
t_pokemon*  pokemonMasCercano (t_entrenador* unEntrenador, t_list* pokemons)
{
	t_pokemon* pokemonFlag = malloc(sizeof(t_pokemon));
	igualarPokemons(pokemonFlag,list_get(pokemons,0));

	uint32_t distanciaDelPrimero = distanciaEntrenadorPokemon(unEntrenador->posicionX,unEntrenador->posicionY,pokemonFlag->posicionX,pokemonFlag->posicionY);

	void elMasCercano(t_pokemon* pokemon){

		uint32_t distanciaTemporal = distanciaEntrenadorPokemon(unEntrenador->posicionX,unEntrenador->posicionY,pokemon->posicionX,pokemon->posicionY);

		if(distanciaTemporal<distanciaDelPrimero){
			igualarPokemons(pokemonFlag,pokemon);
			distanciaDelPrimero = distanciaTemporal;
		}
	}

	list_iterate(pokemons,(void*) elMasCercano);

	return pokemonFlag;
}

t_entrenador* entrenadorAReady(t_list* listaEntrenadores, t_list* listaPokemons)
{
	t_entrenador* entrenadorFlag = malloc(sizeof(t_entrenador));
	igualarEntrenador(entrenadorFlag,list_get(listaEntrenadores,0));
	t_pokemon* pokemonFlag = pokemonMasCercano(entrenadorFlag,listaPokemons);
	entrenadorFlag->pokemonAMoverse = pokemonFlag;

	uint32_t distanciaFlag = distanciaEntrenadorPokemon(entrenadorFlag->posicionX,entrenadorFlag->posicionY,pokemonFlag->posicionX,pokemonFlag->posicionY);

	void procesarEntrenador(t_entrenador* unEntrenador){

		t_pokemon* pokemonTemporal = pokemonMasCercano(unEntrenador,listaPokemons);
		unEntrenador->pokemonAMoverse = pokemonTemporal;
		uint32_t distanciaTemporal = distanciaEntrenadorPokemon(unEntrenador->posicionX,unEntrenador->posicionY,pokemonTemporal->posicionX,pokemonTemporal->posicionY);

		if(distanciaTemporal<distanciaFlag){
			igualarEntrenador(entrenadorFlag,unEntrenador);
			distanciaFlag = distanciaTemporal;
		}
	}

	list_iterate(listaEntrenadores,(void*) procesarEntrenador);

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
	pthread_mutex_lock(&mutexEstadoBloqueado);
	t_list* temporal = list_filter(estado_bloqueado,bloqueadoPorNada);
	pthread_mutex_unlock(&mutexEstadoBloqueado);
	pthread_mutex_lock(&mutexEstadoNew);
	list_add_all(temporal,estado_new);
	pthread_mutex_unlock(&mutexEstadoNew);

	return temporal;
}

bool estaEnLista(t_list* lista, t_entrenador* entrenador){

	bool esElMismo(t_entrenador* unEntrenador){
		return unEntrenador->idEntrenador == entrenador->idEntrenador;
	}

	return list_any_satisfy(lista,(void*) esElMismo);
}

t_list* listaALaQuePertenece(t_entrenador* unEntrenador, pthread_mutex_t* mutexLista){
	if(estaEnLista(estado_new,unEntrenador)){
		*mutexLista = mutexEstadoNew;
		return estado_new;
	}
	else if(estaEnLista(estado_bloqueado,unEntrenador)){
		*mutexLista = mutexEstadoBloqueado;
		return estado_bloqueado;
	}
	else{
		*mutexLista = mutexEstadoReady;
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
        pthread_mutex_lock(&mutexCiclosConsumidos);
        ciclosConsumidos++;
        pthread_mutex_unlock(&mutexCiclosConsumidos);
        unEntrenador->ciclosAcumulados ++;
        log_info(logger,"El entrenador %d se movio a la posicion (%d,%d).",unEntrenador->idEntrenador, unEntrenador->posicionX, unEntrenador->posicionY);
        printf("El entrenador %d se movio a la posicion (%d,%d)\n",unEntrenador->idEntrenador, unEntrenador->posicionX, unEntrenador->posicionY);
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
        pthread_mutex_lock(&mutexCiclosConsumidos);
        ciclosConsumidos++;
        pthread_mutex_unlock(&mutexCiclosConsumidos);
        unEntrenador->ciclosAcumulados ++;
        log_info(logger,"El entrenador %d se movio a la posicion (%d,%d).",unEntrenador->idEntrenador, unEntrenador->posicionX, unEntrenador->posicionY);
        printf("El entrenador %d se movio a la posicion (%d,%d)\n",unEntrenador->idEntrenador, unEntrenador->posicionX, unEntrenador->posicionY);
    }
}

void moverSinDesalojar(t_entrenador* unEntrenador, uint32_t posX, uint32_t posY)
{
    moverEntrenadorX(unEntrenador,posX);
    moverEntrenadorY(unEntrenador,posY);
}

void moverConDesalojoPorRR(t_entrenador* unEntrenador, uint32_t posX, uint32_t posY){

	uint32_t distanciaRecorrida = 0;

	while(distanciaRecorrida < QUANTUM && distanciaEntrenadorPokemon(unEntrenador->posicionX, unEntrenador->posicionY,posX,posY)!=0){

		if(abs(unEntrenador->posicionX - posX) > 0){
		        sleep(retardoCpu);
		        if(unEntrenador->posicionX > posX){
		            unEntrenador->posicionX --;
		        } else {
		            unEntrenador->posicionX ++;
		        }
		        log_info(logger,"El entrenador %d se movio a la posicion (%d,%d).",unEntrenador->idEntrenador, unEntrenador->posicionX, unEntrenador->posicionY);
		        printf("El entrenador %d se movio a la posicion (%d,%d)\n",unEntrenador->idEntrenador, unEntrenador->posicionX, unEntrenador->posicionY);
		        distanciaRecorrida++;
		        pthread_mutex_lock(&mutexCiclosConsumidos);
		        ciclosConsumidos++;
		        pthread_mutex_unlock(&mutexCiclosConsumidos);
		        unEntrenador->ciclosAcumulados ++;
		        if(distanciaRecorrida == QUANTUM)break;
		    }

		if(abs(unEntrenador->posicionY - posY) > 0){
		        sleep(retardoCpu);
		        if(unEntrenador->posicionY > posY){
		            unEntrenador->posicionY --;
		        } else {
		            unEntrenador->posicionY ++;
		        }
		        log_info(logger,"El entrenador %d se movio a la posicion (%d,%d).",unEntrenador->idEntrenador, unEntrenador->posicionX, unEntrenador->posicionY);
		        printf("El entrenador %d se movio a la posicion (%d,%d)\n",unEntrenador->idEntrenador, unEntrenador->posicionX, unEntrenador->posicionY);
		        distanciaRecorrida++;
		        pthread_mutex_lock(&mutexCiclosConsumidos);
		        ciclosConsumidos++;
		        pthread_mutex_unlock(&mutexCiclosConsumidos);
		        unEntrenador->ciclosAcumulados ++;
		        if(distanciaRecorrida == QUANTUM)break;
		    }
	}
}

void moverConDesalojoPorSJF(t_entrenador* unEntrenador, uint32_t posX, uint32_t posY){

	while(distanciaEntrenadorPokemon(unEntrenador->posicionX, unEntrenador->posicionY,posX,posY)!=0){

		pthread_mutex_lock(&mutexEstadoReady);
		if(!esElDeMenorEstimacion(estado_ready,unEntrenador))break;
		pthread_mutex_unlock(&mutexEstadoReady);

		if(abs(unEntrenador->posicionX - posX) > 0){

			sleep(retardoCpu);

			if(unEntrenador->posicionX > posX){
				unEntrenador->posicionX --;
			} else {
				unEntrenador->posicionX ++;
			}
			pthread_mutex_lock(&mutexCiclosConsumidos);
			ciclosConsumidos++;
			pthread_mutex_unlock(&mutexCiclosConsumidos);
			unEntrenador->ciclosAcumulados ++;
			log_info(logger,"El entrenador %d se movio a la posicion (%d,%d).",unEntrenador->idEntrenador, unEntrenador->posicionX, unEntrenador->posicionY);
			printf("El entrenador %d se movio a la posicion (%d,%d)\n",unEntrenador->idEntrenador, unEntrenador->posicionX, unEntrenador->posicionY);
		}

		if(abs(unEntrenador->posicionY - posY) > 0){

			sleep(retardoCpu);

			if(unEntrenador->posicionY > posY){
				unEntrenador->posicionY --;
			} else {
				unEntrenador->posicionY ++;
			}
			pthread_mutex_lock(&mutexCiclosConsumidos);
			ciclosConsumidos++;
			pthread_mutex_unlock(&mutexCiclosConsumidos);
			unEntrenador->ciclosAcumulados ++;
			log_info(logger,"El entrenador %d se movio a la posicion (%d,%d).",unEntrenador->idEntrenador, unEntrenador->posicionX, unEntrenador->posicionY);
			printf("El entrenador %d se movio a la posicion (%d,%d)\n",unEntrenador->idEntrenador, unEntrenador->posicionX, unEntrenador->posicionY);
		}
	}
}

void moverEntrenador(t_entrenador* unEntrenador, uint32_t posX, uint32_t posY)
{
	if(string_equals_ignore_case(ALGORITMO,"FIFO") || string_equals_ignore_case(ALGORITMO,"SJF-SD")){
		moverSinDesalojar(unEntrenador,posX,posY);
	}

	if(string_equals_ignore_case(ALGORITMO,"RR")){
		moverConDesalojoPorRR(unEntrenador,posX,posY);
	}

	if(string_equals_ignore_case(ALGORITMO,"SJF-CD")){
		moverConDesalojoPorSJF(unEntrenador,posX,posY);
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
		bool noEsIgual(char* otroPokemon)
		{
			return !string_equals_ignore_case(otroPokemon,pokemon);
		}
		return list_all_satisfy(unEntrenador->pokesAtrapados,(void*)noEsIgual);
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

t_list* quienesTienenElPokeQueMeFaltaV2(t_entrenador* unEntrenador)
{
	t_list* lista1 = quienesTienenElPokeQueMeFalta(unEntrenador,estado_bloqueado);
	t_list* lista2 = quienesTienenElPokeQueMeFalta(unEntrenador,estado_ready);
	list_add_all(lista2,lista1);
	return lista2;
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
	uint32_t indiceDelPokemonDondeEstaEnElEntrenador2 = retornarIndice(entrenador2->pokesAtrapados,pokemon(pokemonesQueLeFaltan(entrenador1)));

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

	//pthread_mutex_lock(&mutexPokemonsRecibidos);
	t_entrenador* entrenadorInicial = (t_entrenador*) list_get(entrenadores,0);
	//pthread_mutex_unlock(&mutexPokemonsRecibidos);
	double estimacionFlag = entrenadorInicial->estimacion;

	void buscarMenor(t_entrenador* unEntrenador){
		double estimacion = unEntrenador->estimacion;
		if(estimacion < estimacionFlag){
			estimacionFlag = estimacion;
		}
	}

	list_iterate(entrenadores,(void*) buscarMenor);

	bool tieneLaMenor(t_entrenador* unEntrenador){
		return fabs(unEntrenador->estimacion) == fabs(estimacionFlag);
	}

	return list_remove_by_condition(entrenadores,(void*)tieneLaMenor);
}

bool esElDeMenorEstimacion(t_list* entrenadores, t_entrenador* entrenador){

	double estimacionFlag = entrenador->estimacion;

	void buscarMenor(t_entrenador* unEntrenador){
		double estimacion = unEntrenador->estimacion;
		if(estimacion < estimacionFlag){
			estimacionFlag = estimacion;
		}
	}

	list_iterate(entrenadores,(void*) buscarMenor);

	bool tieneLaMenor(t_entrenador* unEntrenador){
		return fabs(unEntrenador->estimacion)== fabs(estimacionFlag);
	}

	return tieneLaMenor(entrenador);
}

void mostrarCiclos(t_entrenador* entrenador){
	printf("Los ciclos de CPU consumidos por el entrenador %d son: %d\n",entrenador->idEntrenador,entrenador->ciclosAcumulados);
}







