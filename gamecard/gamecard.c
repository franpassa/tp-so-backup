#include "utils/gamecard.h"

<<<<<<< HEAD
void inicializar(){


=======
void asignar_globales(){
	config = get_config(CONFIG_PATH);
	logger = crear_log(LOG_PATH);
	pthread_mutex_init(&mutex_bitmap, NULL);
}

void inicializar(){

	asignar_globales();
>>>>>>> 7a3b519ce534f7b5e92b4860df42e87b899b2ff0
	char* punto_montaje = config_get_string_value(config, "PUNTO_MONTAJE_TALLGRASS");
	fspaths = init_fspaths(punto_montaje);
	init_fs();
}

<<<<<<< HEAD
void asignar_globales(){
	config = get_config(CONFIG_PATH);
	logger = crear_log(LOG_PATH);
	pthread_mutex_init(&mutex_bitmap, NULL);
}
=======

>>>>>>> 7a3b519ce534f7b5e92b4860df42e87b899b2ff0

void terminar(){
	log_destroy(logger);
	config_destroy(config);
	free_fspaths(fspaths);
}

<<<<<<< HEAD
=======
void print_string(char* string){
	printf("%s\n", string);
}

>>>>>>> 7a3b519ce534f7b5e92b4860df42e87b899b2ff0
int main(){

	inicializar();

	/*pthread_create(&hilo_gameboy, NULL, (void*) escuchar_gameboy, NULL);
	pthread_join(hilo_gameboy, NULL);*/
<<<<<<< HEAD
	t_list* bloques = list_create();
	agregar_a_lista(bloques, 0);
	agregar_a_lista(bloques, 1);
	agregar_a_lista(bloques, 2);
	char* contenido = get_blocks_content(bloques);
	t_list* lista_coordenadas = string_to_coordenadas(contenido);
=======
//	t_list* bloques = list_create();
//	agregar_a_lista(bloques, 0);
//	agregar_a_lista(bloques, 1);
//	agregar_a_lista(bloques, 2);
//	char* contenido = get_blocks_content(bloques);
//	t_list* lista_coordenadas = string_to_coordenadas(contenido);
>>>>>>> 7a3b519ce534f7b5e92b4860df42e87b899b2ff0

	terminar();
	return EXIT_SUCCESS;
}
