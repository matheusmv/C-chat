
client:
	gcc cliente.c -lpthread -o cliente


server:
	gcc servidor.c -lpthread -o servidor


clean:
	rm cliente
	rm servidor
