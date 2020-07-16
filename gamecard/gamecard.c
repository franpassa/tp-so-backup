#include "utils/gamecard.h"

void init_semaforos(){
	pthread_mutex_init(&mutex_dict, NULL);
	pthread_mutex_init(&mutex_bitmap, NULL);
	pthread_mutex_init(&mutex_reconexion, NULL);
	pthread_mutex_init(&mutex_espera, NULL);
	pthread_cond_init(&cond_reconectado, NULL);
}

void init_globales(){
	config = get_config(CONFIG_PATH);
	logger = crear_log(LOG_PATH);
	sem_files = dictionary_create();
	init_semaforos();
}

void inicializar(){
	init_globales();

	char* punto_montaje = config_get_string_value(config, "PUNTO_MONTAJE_TALLGRASS");
	fspaths = init_fspaths(punto_montaje);
	init_fs();
}

void terminar(){
	log_destroy(logger);
	config_destroy(config);
	free_fspaths(fspaths);
}

int main(){

	inicializar();

	pthread_t hilo_gameboy;
	pthread_create(&hilo_invocador, NULL, (void*) iniciar_hilos_escucha_broker, NULL);
	pthread_detach(hilo_invocador);
	pthread_create(&hilo_gameboy, NULL, (void*) escuchar_gameboy, NULL);
	pthread_join(hilo_gameboy, NULL);

	terminar();
	return EXIT_SUCCESS;
}
