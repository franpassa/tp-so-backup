#ifndef _CONEXIONES_TEAM_H_
#define _CONEXIONES_TEAM_H_

#include "team.h"

void enviar_gets(t_list* objetivos_globales);
int suscribirse_a_cola(queue_name cola,char* ip_broker, char* puerto_broker);

#endif /* _CONEXIONES_TEAM_H_*/
