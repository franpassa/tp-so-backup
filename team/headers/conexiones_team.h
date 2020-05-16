#ifndef _CONEXIONES_TEAM_H_
#define _CONEXIONES_TEAM_H_

#include "team.h"

void enviar_gets(t_list* objetivos_globales);
int suscribirse_a_queue(queue_name cola,char* ip_broker, char* puerto_broker);
void recibirLocalized();
void esperar_cliente(int* socket_servidor);
void estado_exec();
void algoritmoFifo(t_entrenador* entrenador);

#endif /* _CONEXIONES_TEAM_H_*/
