all: ipk-client ipk-server

ipk-client: ipk-client.c
	gcc -w ipk-client.c -o ipk-client

ipk-server: ipk-server.c
	gcc -w ipk-server.c -o ipk-server