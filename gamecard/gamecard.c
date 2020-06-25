#include "utils/gamecard.h"

void asignar_globales(){
	config = get_config(CONFIG_PATH);
	logger = crear_log(LOG_PATH);
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
	t_pokemon pikachu = init_pokemon("pikachu", 10, 56, 10000); // REVISAR ESPACIO EN BLANCO EN BLOQUE 3

	if(existe_pokemon(pikachu.nombre)){
		int bytes_escritos = agregar_posicion_pokemon(pikachu);
	} else {
		crear_pokemon(pikachu);
	}

	terminar();
	return EXIT_SUCCESS;
}
