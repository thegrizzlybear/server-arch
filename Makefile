all: server1/server.c server2/server.c server3/server.c server4/server.c client.c
	gcc server1/server.c -o server1/server
	gcc server2/server.c -o server2/server
	gcc server3/server.c -o server3/server -pthread 
	gcc server4/server.c -o server4/server
	gcc --no-warning client.c -o client

clean:
	rm client server1/server server2/server server3/server server4/server
