#ifndef ENTRENADOR_H_
#define ENTRENADOR_H_

#include "pokemon.h"
#include <math.h>
#include <pthread.h>

typedef enum{
	ESPERA_CAUGHT,
	MOTIVO_NADA,
	ESPERA_DEADLOCK,
	ESTADO_EXIT,
	RESOLVIENDO_DEADLOCK,
}motivo;

typedef struct Entrenador
{
	uint32_t posicionX;
	uint32_t posicionY;
	t_list* pokesAtrapados;
	t_list* pokesObjetivos;
	uint32_t idEntrenador;
	uint32_t idRecibido;
	t_pokemon* pokemonAMoverse;
	motivo motivoBloqueo;
	double estimacion;
	uint32_t ciclosAcumulados;
	uint32_t posXAMoverse;
	uint32_t posYAMoverse;
}t_entrenador;

t_log* logger;

// entrenadores en new
t_list* estado_new;
// entrenadores en block
t_list* estado_bloqueado;
t_list* estado_ready;

pthread_mutex_t mutexLogEntrenador;

char* ALGORITMO;
uint32_t QUANTUM;
double ESTIMACION_INICIAL;
double ALPHA;
uint32_t retardoCpu;
uint32_t ciclosConsumidos;
uint32_t cambiosDeContexto;
uint32_t cantidadDeadlocks;

// posiciones
uint32_t posicionXEntrenador(int nroEntrenador, char** posicionesEntrenadores);
uint32_t posicionYEntrenador(int nroEntrenador, char** posicionesEntrenadores);

//cantidades
uint32_t cantidadTotalEntrenadores (char** posicionesEntrenadores);
uint32_t distanciaEntrenadorPokemon(uint32_t posXEntrenador , uint32_t posYEntrenador, uint32_t posXPokemon, uint32_t PosYPokemon);
/*
 * las funciones  el entrenadorMasCercanoAUnPOkemonDeLaLista y
 * elQueEstaMasCerca estan definidas en Team.h porque aca no reconocen el tipo
 * t_pokemon.
 * */

//liberaciones + muestra
void liberarArray(char** posicionesEntrenadores);
void liberarEntrenador(void* entrenador);
void mostrarEntrenador(void* entrenador);
void mostrarCiclos(t_entrenador* entrenador);

// listas nuevas / modificar listas
t_list* insertarPokesEntrenador(uint32_t nroEntrenador, t_list* pokemons, char** pokesEntrenadores);
t_list* crearListaDeEntrenadores(char** posicionesEntrenadores, char** pokesEntrenadores, char** pokesObjetivos);
t_list* crearListaPokesObjetivos(t_list* entrenadores);


t_pokemon*  pokemonMasCercano (t_entrenador* unEntrenador, t_list* pokemons);
t_entrenador* entrenadorAReady(t_list* listaEntrenadores, t_list* listaPokemons);


// listado de entrenadores new + block x motivo_nada
t_list* todosLosEntrenadoresAPlanificar();


bool bloqueadoPorNada(void* unEntrenador);
bool bloqueadoPorDeadlock(t_entrenador* unEntrenador);
bool estaEnLista(t_list* lista, t_entrenador* unEntrenador);
t_list* listaALaQuePertenece(t_entrenador* unEntrenador);

bool puedeAtrapar(t_entrenador* entrenador);
void mostrar_ids(void* id);
void moverEntrenadorX(t_entrenador* unEntrenador, uint32_t posX);
void moverEntrenadorY(t_entrenador* unEntrenador, uint32_t posY);
void moverEntrenador(t_entrenador* unEntrenador, uint32_t posX, uint32_t posY);
void moverSinDesalojar(t_entrenador* unEntrenador, uint32_t posX, uint32_t posY);
void moverEntrenador(t_entrenador* unEntrenador, uint32_t posX, uint32_t posY);
void moverConDesalojoPorRR(t_entrenador* unEntrenador, uint32_t posX, uint32_t posY);
void moverConDesalojoPorSJF(t_entrenador* unEntrenador, uint32_t posX, uint32_t posY);

t_list* pokemonesAlPedo(t_entrenador* unEntrenador);
t_list* pokemonesQueLeFaltan(t_entrenador* unEntrenador);
t_list* pokemonsQueLeSobranDistintos(t_entrenador* unEntrenador);
t_list* pokemonsRepetidos(t_entrenador* unEntrenador);
t_list* pokemonsQueLogroAtrapar(t_entrenador* unEntrenador);
uint32_t cantidadAtrapadosDeMas(t_entrenador* entrenador,char* unaEspecie);
t_list* quienesTienenElPokeQueMeFalta(t_entrenador* unEntrenador, t_list* lista);
uint32_t retornarIndice(t_list* lista, char* nombre);
void realizarCambio(t_entrenador* entrenador1, t_entrenador* entrenador2);


t_list* crearListaObjetivosPosta(t_list* pokesObjetivosGlobal, t_list* entrenadoresNew);
void sacar1(char* nombre, t_list* listaDeEspecies);
void recalcularEstimacion(t_entrenador* entrenador,uint32_t ciclosRecorridos);
t_entrenador* elDeMenorEstimacion(t_list* entrenadores);
bool esElDeMenorEstimacion(t_list* entrenadores, t_entrenador* entrenador);


t_list* quienesTienenElPokeQueMeFaltaV2(t_entrenador* unEntrenador);


#endif /* ENTRENADOR_H_ */
