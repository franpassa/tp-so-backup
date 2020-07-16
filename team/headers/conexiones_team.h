#ifndef _CONEXIONES_TEAM_H_
#define _CONEXIONES_TEAM_H_

#include "team.h"

void enviar_gets(t_list* objetivos_globales);
void recibirLocalized();
void recibirCaught();
void recibirAppeared();
void esperar_cliente(int* socket_servidor);
void estado_exec();
void pasar_a_ready();
void planificacion(t_list* lista);
bool necesitoElMensaje(uint32_t id); // Chequea si el id esta en la lista de ids_recibidos;
void* buscarEntrenador(uint32_t id); // Buscar el entrenador que tiene ese idRecibido;
void cambiarEstado(t_entrenador* entrenador);
void invocarHiloReconexion();
void conectarABroker();
bool todasLasColasEstanVacias();
void deadlock();
void cambiarAExit(t_entrenador* entrenador);
void cambiarABlockNada(t_entrenador* entrenador);
void cambiarABlockDeadlock(t_entrenador* entrenador);
void resolverDeadlock();


#endif /* _CONEXIONES_TEAM_H_*/
