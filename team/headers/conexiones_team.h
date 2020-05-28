#ifndef _CONEXIONES_TEAM_H_
#define _CONEXIONES_TEAM_H_

#include "team.h"

void enviar_gets(t_list* objetivos_globales);
int suscribirse_a_queue(queue_name cola,char* ip_broker, char* puerto_broker);
void recibirLocalized();
void recibirCaught();
void esperar_cliente(int* socket_servidor);
void estado_exec();
void pasar_a_ready();
void algoritmoFifo(t_entrenador* entrenador);
bool necesitoElMensaje(uint32_t id); // Chequea si el id esta en la lista de ids_recibidos;
void* buscarEntrenador(uint32_t id); // Buscar el entrenador que tiene ese idRecibido;
void cambiarEstado(t_entrenador* entrenador);

#endif /* _CONEXIONES_TEAM_H_*/
