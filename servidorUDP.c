#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ctype.h>

#define MAX_SIZE 1000
/* Función para imprimir errores */
void error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    /* Comprobar número de argumentos */
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <puerto> \n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);

    char mensaje [MAX_SIZE];

    if (port <= IPPORT_USERRESERVED || port > 65535) {
        fprintf(stderr, "Numero de puerto invalido: %s\n", argv[1]);
        printf("Escuchando al puerto por defecto: 8000");
        port = 8000;   
    }

    /* Creamos el socket, protocolo 0 para cada par dominio/tipo */
    int receptor_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (receptor_fd < 0) error("Error en creacion del Socket");


    /* Configurar address del servidor */
    struct sockaddr_in socket_propio;
    struct sockaddr_in socket_remoto;
    size_t sizeMssg = MAX_SIZE;
    
    socket_propio.sin_family = AF_INET;
    socket_propio.sin_addr.s_addr = INADDR_ANY; /* Escuchar a todas las interfaces */
    socket_propio.sin_port = htons(port); /* Traducir orden de máquina a orden de red */

    /* Bind del socket a la address, porque cada socket tiene que tener una dirección */
    if (bind(receptor_fd, (struct sockaddr *)&socket_propio, sizeof(socket_propio)) < 0)
        error("Bind fallo");

    while (1) {
        socklen_t tamSocketRemoto = sizeof(socket_remoto);
        ssize_t bytesRecv = recvfrom(receptor_fd, mensaje, sizeMssg, 0, (struct sockaddr *)&socket_remoto, &tamSocketRemoto);
        if (bytesRecv < 0)
            error("recvfrom fallo");
        
        
        int remoto_port = ntohs(socket_remoto.sin_port);
        char remoto_ip[INET_ADDRSTRLEN];

        mensaje[bytesRecv] = '\0';

        //Se convierte la línea recibida a mayúsculas
        for (int i = 0; i < bytesRecv; i++) {
            mensaje[i] = toupper((unsigned char) mensaje[i]);
        }

        //Se envían datos convertidos de vuelta al cliente y se comprueban errores
        ssize_t bytes_enviados = sendto(receptor_fd, mensaje, bytesRecv, 0, (struct sockaddr *)&socket_remoto, tamSocketRemoto);
        if (bytes_enviados == -1) {
            perror("Error enviando datos al cliente");
        } else if (bytes_enviados != bytesRecv) {
            printf("Error: No se enviaron todos los bytes (%zd de %zd)\n", bytes_enviados, bytesRecv);
        } 
        printf("Enviados %zd bytes convertidos a mayúsculas\n", bytes_enviados);
        
        int tamMens =  bytesRecv / sizeof(mensaje[0]);
        
        printf("Bytes recibidos: %ld\n", bytesRecv);
        /* Convertimos una IP binaria en orden de red a IP legible */
        inet_ntop(AF_INET, &socket_remoto.sin_addr, remoto_ip, INET_ADDRSTRLEN);
        printf("Receptor recibió un mensaje en el puerto %d IP %s...\n", remoto_port, remoto_ip);
        printf("Mensaje: %s\n", mensaje);
        printf("Número de chars: %d\n", tamMens);
    }

    /* Cerramos la conexión del socket del servidor */
    close(receptor_fd);
    return 0;
}
