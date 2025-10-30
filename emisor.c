#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#define N_FLOATS 1000
/* Función para imprimir errores */
void error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    /* Comprobar número de argumentos */
    if (argc != 4) {
        fprintf(stderr, "Uso: %s <puerto_propio> <ip_destino> <puerto_destino> \n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int puerto_propio = atoi(argv[1]);
    int puerto_destino = atoi(argv[3]);
    char* ip_destino = argv[2]; 

    float mensaje [N_FLOATS];

    size_t sizeMssg = sizeof(mensaje);
    
    for (int i = 0; i < N_FLOATS; ++i)
    {
        mensaje[i] = 2.5 * (i+2);
        printf("Mensaje[%d]: %f\n", i, mensaje[i]);
    }

    if (puerto_propio <= IPPORT_USERRESERVED || puerto_propio > 65535) {
        fprintf(stderr, "Numero de puerto_propio invalido: %s\n", argv[1]);
        printf("Escuchando al puerto por defecto: 8000");
        puerto_propio = 7000;   
    }

    if (puerto_destino <= IPPORT_USERRESERVED || puerto_destino > 65535) {
        fprintf(stderr, "Numero de puerto_destino invalido: %s\n", argv[1]);
        printf("Escuchando al puerto por defecto: 8000");
        puerto_destino = 8000;   
    }

    /* Creamos el socket, protocolo 0 para cada par dominio/tipo */
    int emisor_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (emisor_fd < 0) error("Error en creacion del Socket");

    /* Configurar address del servidor */
    struct sockaddr_in socket_propio;
    struct sockaddr_in socket_remoto;
    struct in_addr socket_remoto2;

    socket_propio.sin_family = AF_INET;
    socket_propio.sin_addr.s_addr = INADDR_ANY; /* Escuchar a todas las interfaces */
    socket_propio.sin_port = htons(puerto_propio); /* Traducir orden de máquina a orden de red */

    socket_remoto.sin_family = AF_INET;
    int retIP = inet_pton(AF_INET, ip_destino, &socket_remoto2); /* Escuchar a todas las interfaces */
    socket_remoto.sin_addr.s_addr = socket_remoto2.s_addr;
    if (retIP < 0) error("inet_pton fallo");
    socket_remoto.sin_port = htons(puerto_destino); /* Traducir orden de máquina a orden de red */

    /* Bind del socket a la address, porque cada socket tiene que tener una dirección */
    if (bind(emisor_fd, (struct sockaddr *)&socket_propio, sizeof(socket_propio)) < 0)
        error("Bind fallo");

    socklen_t tamSocketRemoto = sizeof(socket_remoto);
    ssize_t bytesEnv = sendto(emisor_fd, mensaje, sizeMssg, 0, (struct sockaddr *)&socket_remoto, tamSocketRemoto);
    if (bytesEnv < 0) error("sendto fallo");
    
    printf("Bytes enviados: %ld\n", bytesEnv);

    /* Cerramos la conexión del socket del servidor */
    close(emisor_fd);
    return 0;
}
