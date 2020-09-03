
client:
	gcc -c ./cliente/cliente.c -o ./cliente/cliente.o
	gcc ./cliente/main-cliente.c ./cliente/cliente.o -I ./cliente -o ./cliente/cliente


server:
	gcc -c ./servidor/servidor.c -o ./servidor/servidor.o
	gcc ./servidor/main-servidor.c ./servidor/servidor.o -I ./servidor -lpthread -o ./servidor/servidor


clean:
	rm ./cliente/*.o
	rm ./cliente/cliente
	rm ./servidor/*.o
	rm ./servidor/servidor
