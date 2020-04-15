#include "../team_log.h"

t_log* iniciar_logger(void) {
	return log_create(PATH_LOG, PROGRAM_NAME, 1, LOG_LEVEL_INFO);
}
