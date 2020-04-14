#include "../team_config.h"

t_config* leer_config(void) {
	return config_create(PATH_CONFIG);
}