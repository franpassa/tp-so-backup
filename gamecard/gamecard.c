#include "utils/gamecard.h"

void asignar_globales(){
	config = get_config(CONFIG_PATH);
	logger = crear_log(LOG_PATH);
	sem_files = dictionary_create();
	pthread_mutex_init(&mutex_dict, NULL);
	pthread_mutex_init(&mutex_bitmap, NULL);
}

void inicializar(){
	asignar_globales();

	char* punto_montaje = config_get_string_value(config, "PUNTO_MONTAJE_TALLGRASS");
	fspaths = init_fspaths(punto_montaje);
	init_fs();
}

void terminar(){
	log_destroy(logger);
	config_destroy(config);
	free_fspaths(fspaths);
}

void print_int(int* string){
	printf("%d\n", *string);
}

int main(){

	inicializar();

	pthread_t hilo_gameboy;
	pthread_create(&hilo_gameboy, NULL, (void*) escuchar_gameboy, NULL);
	pthread_join(hilo_gameboy, NULL);

	terminar();
	return EXIT_SUCCESS;
}
