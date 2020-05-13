#include "../conexiones_team.h"


void enviar_gets(t_list* objetivos_globales){

	void enviar(void* especie)
	{
		/*IMPORTO EL IP Y EL PUERTO DEL BROKER */
		char* IP = config_get_string_value(config,"IP_BROKER");
		char* PUERTO = config_get_string_value(config, "PUERTO_BROKER");
		/*CREO CONEXION*/
		int socket_para_envio = conectar_a_broker(PRODUCTOR,IP,PUERTO); // ESTA FUNCION ESTA EN DUDA, PORQUE LA COLA PRODUCTOR NO CREO QUE SE MANTENGA
		if(socket_para_envio == -1){
			printf("Fallo la conexion con el Broker.\n");
		}
		/*CREO EL MENSAJE Y LO ENVIO*/
		char* mensaje = ((t_especie*)especie)->especie;
		get_pokemon_msg* a_enviar = get_msg(mensaje);
		int id_respuesta = enviar_mensaje(GET_POKEMON,a_enviar,socket_para_envio);
		/*AGREGO EL ID A LA LISTA DE IDS ENVIADOS*/
		list_add(ids_get,&id_respuesta);
		/*CIERRO CONEXION*/
		close(socket_para_envio);
	}

	list_iterate(objetivos_globales,enviar);
}

int suscribirse_a_cola(queue_name cola,char* ip_broker, char* puerto_broker){
	int socket_cola;

	if(cola == APPEARED_POKEMON){
		socket_cola = conectar_a_broker(APPEARED_POKEMON,ip_broker,puerto_broker);
		if(socket_cola == -1){
			printf("Error al suscribirse a la cola 'APPEARED POKEMON'.\n\n");
			return socket_cola;
		}
	}else if(cola == CAUGHT_POKEMON){
		socket_cola = conectar_a_broker(CAUGHT_POKEMON,ip_broker,puerto_broker);
		if(socket_cola == -1){
			printf("Error al suscribirse a la cola 'CAUGHT POKEMON'.\n\n");
			return socket_cola;
		}
	}else if(cola == LOCALIZED_POKEMON){
		socket_cola = conectar_a_broker(LOCALIZED_POKEMON,ip_broker,puerto_broker);
		if(socket_cola == -1){
			printf("Error al suscribirse a la cola 'LOCALIZED POKEMON'.\n\n");
			return socket_cola;
		}
	}
	return socket_cola;
}

