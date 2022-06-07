main:
	gcc -std=c99 -pthread -o client client.c
	gcc -std=c99 -pthread -o server server.c


clean:
	rm -f client
	rm -f server

