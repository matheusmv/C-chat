all:
	gcc client.c -lpthread -o client
	gcc server.c -lpthread -o server


client:
	gcc client.c -lpthread -o client


server:
	gcc server.c -lpthread -o server


clean:
	rm client
	rm server
