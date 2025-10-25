all:
	gcc -o emisor emisor.c -Wall
	gcc -o receptor receptor.c -Wall
	gcc -o clienteUDP clienteUDP.c -Wall
	gcc -o servidorUDP servidorUDP.c -Wall