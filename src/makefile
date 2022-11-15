all: client.o server.o
		g++ client.o -o client
		g++ server.o -o server
client.o: client.cpp
		g++ -c client.cpp -o client.o
server.o: server.cpp
		g++ -c server.cpp -o server.o
clean:
		rm *.o client server
