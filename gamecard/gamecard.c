#include "utils/gamecard.h"

void inicializar(){


	char* punto_montaje = config_get_string_value(config, "PUNTO_MONTAJE_TALLGRASS");
	fspaths = init_fspaths(punto_montaje);
	init_fs();
}

void asignar_globales(){
	config = get_config(CONFIG_PATH);
	logger = crear_log(LOG_PATH);
	pthread_mutex_init(&mutex_bitmap, NULL);
}

void terminar(){
	log_destroy(logger);
	config_destroy(config);
	free_fspaths(fspaths);
}

int main(){

	inicializar();

	/*pthread_create(&hilo_gameboy, NULL, (void*) escuchar_gameboy, NULL);
	pthread_join(hilo_gameboy, NULL);*/
	t_list* bloques = list_create();
	agregar_a_lista(bloques, 0);
	agregar_a_lista(bloques, 1);
	agregar_a_lista(bloques, 2);
	char* contenido = get_blocks_content(bloques);
	t_list* lista_coordenadas = string_to_coordenadas(contenido);

	terminar();
	return EXIT_SUCCESS;
}
