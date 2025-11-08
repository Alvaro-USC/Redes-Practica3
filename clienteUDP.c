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

/* Convertimos el nombre del archivo a mayúsculas */
char* to_uppercase_filename(const char *filename) {
    char *upper_filename = strdup(filename);

    if (!upper_filename) error("Memory allocation failed");
    
    for (int i = 0; upper_filename[i]; i++) {
        upper_filename[i] = toupper(upper_filename[i]);
    }
    return upper_filename;
}

int main(int argc, char *argv[]) {
    /* Comprobar número de argumentos */
    if (argc != 5) {
        fprintf(stderr, "Uso: %s <puerto_propio> <ip_destino> <puerto_destino> <archivo_texto>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int puerto_propio = atoi(argv[1]);
    int puerto_destino = atoi(argv[3]);
    char* ip_destino = argv[2]; 
    char* archivo = argv[4];

    FILE *fichero_entrada = fopen(archivo, "r");
    if (fichero_entrada == NULL) error("Error al abrir archivo de entrada");


    char mensaje[MAX_SIZE];
    char buffer_respuesta[MAX_SIZE];

    size_t sizeMssg = sizeof(mensaje);

    if (puerto_propio <= IPPORT_USERRESERVED || puerto_propio > 65535) {
        fprintf(stderr, "Numero de puerto_propio invalido: %s\n", argv[1]);
        printf("Escuchando al puerto por defecto: 7000\n");
        puerto_propio = 7000;   
    }

    if (puerto_destino <= IPPORT_USERRESERVED || puerto_destino > 65535) {
        fprintf(stderr, "Numero de puerto_destino invalido: %s\n", argv[1]);
        printf("Escuchando al puerto por defecto: 8000\n");
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

    socklen_t tamSocketRemoto = sizeof(socket_remoto);
    ssize_t bytesEnv = 0;

    /* Bind del socket a la address, porque cada socket tiene que tener una dirección */
    if (bind(emisor_fd, (struct sockaddr *)&socket_propio, sizeof(socket_propio)) < 0){
        error("Bind fallo");  
    }

    char* nombre_fichero = to_uppercase_filename(archivo);
    FILE *fichero_salida = fopen(nombre_fichero, "w");
    if (fichero_salida == NULL) error("Error al abrir archivo de salida");
    while (fgets(mensaje, MAX_SIZE, fichero_entrada) != NULL) {
        //Se envía la línea al servidor
        size_t len = strlen(mensaje);
        bytesEnv = sendto(emisor_fd, mensaje, len, 0, (struct sockaddr *)&socket_remoto, tamSocketRemoto);
        if (bytesEnv == -1) {
            perror("Error enviando datos al servidor");
            break;
        } else if ((size_t)bytesEnv != len) {
            printf("Error: No se enviaron todos los bytes (%zd de %zu)\n", bytesEnv, strlen(mensaje));
            break;
        }

        
        //Se recibe la respuesta del servidor
        bytesEnv = recvfrom(emisor_fd, buffer_respuesta, sizeMssg, 0, (struct sockaddr *)&socket_remoto, &tamSocketRemoto);
        if (bytesEnv == -1) {
            perror("Error recibiendo datos del servidor");
            break;
        } else if (bytesEnv == 0) {
            printf("El servidor cerró la conexión\n");
            break;
        }            
        //Se asegura la terminación nula del buffer
        buffer_respuesta[bytesEnv] = '\0';
        //Se escribe la respuesta en el archivo de salida
        if (fputs(buffer_respuesta, fichero_salida) == EOF) {
            perror("Error escribiendo en archivo de salida");
            break;
        }
        
    }

    if (bytesEnv < 0) error("sendto fallo");
    
    printf("Bytes enviados: %ld\n", bytesEnv);

    /* Cerramos la conexión del socket del servidor */
    fclose(fichero_salida);
    fclose(fichero_entrada);
    free(nombre_fichero);
    close(emisor_fd);
    return 0;
}
