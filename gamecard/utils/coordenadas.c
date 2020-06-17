#include "gamecard.h"

t_coordenada string_to_coordenada(char* string_coordenada){
	uint32_t x, y, cantidad;
	int largo_string = string_length(string_coordenada);
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

char* coordenadas_to_string(t_list* coordenadas){
	char* coordenadas_string = string_new();
	int coordenadas_size = list_size(coordenadas);

	for(int i = 0; i < coordenadas_size; i++){
		t_coordenada* coordenada = list_get(coordenadas, i);
		string_append_with_format(&coordenadas_string, "%d-%d=%d\n", coordenada->x, coordenada->y, coordenada->cantidad);
	}
	int largo_string = string_length(coordenadas_string);
	coordenadas_string[largo_string-1] = '\0'; // Saco el último \n

	return coordenadas_string;
}

// Si ya existe en la lista le suma 1 a cantidad, si no lo agrega.
void add_coordenada(t_list* lista_coordenadas, t_coordenada coordenada){

	bool es_la_misma(t_coordenada* coordenada_en_lista){
		bool mismaX = coordenada.x == coordenada_en_lista->x;
		bool mismaY = coordenada.y == coordenada_en_lista->y;
		return mismaX && mismaY;
	}

	t_coordenada* misma_coordenada = (t_coordenada*) list_find(lista_coordenadas, es_la_misma);

	if(misma_coordenada != NULL){
		misma_coordenada->cantidad++;
	} else {
		t_coordenada* nueva_coordenada = malloc(sizeof(t_coordenada));
		*nueva_coordenada = coordenada;
		list_add(lista_coordenadas, nueva_coordenada);
	}
}

t_coordenada* find_coordenada(t_list* lista_coordenadas, t_coordenada coordenada){

	bool es_la_misma(t_coordenada* coordenada_en_lista){
		bool mismaX = coordenada.x == coordenada_en_lista->x;
		bool mismaY = coordenada.y == coordenada_en_lista->y;
		return mismaX && mismaY;
	}

	return (t_coordenada*) list_find(lista_coordenadas, es_la_misma);
}
