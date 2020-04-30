## Flujo suscripción (ejemplo GAME_CARD a BROKER)

1- broker escucha nuevas conexiones
2- game_card envia un 'connect'
3- broker recibe el connect, obtiene numero de socket y se queda esperando numero de cola
4- game_card envía numero de cola y espera confirmación
5- broker recibe número de cola, asocia el socket a esa cola y devuelve confirmación (0 OK / -1 NO OK)
6- game_card recibe confirmación y continúa...
