#ifndef _CONEXIONES_TEAM_H_
#define _CONEXIONES_TEAM_H_

#include "team.h"

typedef enum
{
	MENSAJE = 1,
}op_code;

int crear_conexion(char* ip, char* puerto);

#endif /* _CONEXIONES_TEAM_H_*/
