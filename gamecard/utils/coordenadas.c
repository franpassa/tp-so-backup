#include "gamecard.h"

t_coordenada string_to_coordenada(char* string_coordenada){
	uint32_t x, y, cantidad;
	char* template_scan = string_duplicate("%d-%d=%d");

	sscanf(string_coordenada, template_scan, &x, &y, &cantidad);

	return init_coordenada(x, y, cantidad);
}

t_list* string_to_coordenadas(char* string_coordenadas){
	t_list* coordenadas = list_create();

	char** array_coordenadas = string_split(string_coordenadas, "\n");
	char* coordenada = NULL;
	int index = 0;
	while((coordenada = array_coordenadas[index]) != NULL){
		t_coordenada* struct_coordenada = malloc(sizeof(t_coordenada));
		*struct_coordenada = string_to_coordenada(coordenada);
		list_add(coordenadas, struct_coordenada);
		index++;
	}
	free(array_coordenadas);

	return coordenadas;
}

char* coordenada_to_string(t_coordenada coordenada){
	char* string_coordenada = string_new();
	string_append_with_format(&string_coordenada, "%d-%d=%d", coordenada.x, coordenada.y, coordenada.cantidad);

	return string_coordenada;
}

char* coordenadas_to_string(t_list* coordenadas){
	char* coordenadas_string = string_new();
	int coordenadas_size = list_size(coordenadas);

	for(int i = 0; i < coordenadas_size; i++){
		t_coordenada* coordenada = list_get(coordenadas, i);
		string_append_with_format(&coordenadas_string, "%s\n", coordenada_to_string(*coordenada));
	}
	int largo_string = string_length(coordenadas_string);
	coordenadas_string[largo_string-1] = '\0'; // Saco el último \n

	return coordenadas_string;
}

// Si ya existe en la lista le suma la cantidad, si no lo crea y lo agrega.
void add_coordenada(t_list* lista_coordenadas, t_coordenada coordenada){

	t_coordenada* coordenada_en_lista = find_coordenada(lista_coordenadas, coordenada);

	if(coordenada_en_lista != NULL){
		coordenada_en_lista->cantidad += coordenada.cantidad;
	} else {
		t_coordenada* nueva_coordenada = malloc(sizeof(t_coordenada));
		*nueva_coordenada = coordenada;
		list_add(lista_coordenadas, nueva_coordenada);
	}
}

bool quitar_de_coordenada(t_list* lista_coordenadas, t_coordenada coordenada){

	t_coordenada* coordenada_en_lista = find_coordenada(lista_coordenadas, coordenada);

	if(coordenada_en_lista != NULL){
		coordenada_en_lista->cantidad -= coordenada.cantidad;
		if(coordenada_en_lista->cantidad == 0){
			remover_coordenada(lista_coordenadas, coordenada);
		}
	}
}

void remover_coordenada(t_list* lista_coordenadas, t_coordenada coordenada){

	bool es_la_misma(t_coordenada* coordenada_en_lista){
		bool mismaX = coordenada.x == coordenada_en_lista->x;
		bool mismaY = coordenada.y == coordenada_en_lista->y;
		return mismaX && mismaY;
	}

	list_remove_and_destroy_by_condition(lista_coordenadas, (void*) es_la_misma, free);

}

t_coordenada* find_coordenada(t_list* lista_coordenadas, t_coordenada coordenada){

	bool es_la_misma(t_coordenada* coordenada_en_lista){
		bool mismaX = coordenada.x == coordenada_en_lista->x;
		bool mismaY = coordenada.y == coordenada_en_lista->y;
		return mismaX && mismaY;
	}

	return (t_coordenada*) list_find(lista_coordenadas, (void*) es_la_misma);
}
