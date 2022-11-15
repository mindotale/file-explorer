all: buffer.o client.o server.o
	g++ buffer.o client.o -o client
	g++ buffer.o server.o -o server
	rm -f *.o

client: buffer.o client.o
	g++ buffer.o client.o -o client

server: buffer.o server.o
	g++ buffer.o server.o -o server

buffer.o:
	g++ -c buffer.cpp -o buffer.o -std=c++11

client.o:
	g++ -c client.cpp -o client.o -std=c++11

server.o:
	g++ -c server.cpp -o server.o -std=c++11

clean:
	rm -f client.o
	rm -f server.o
	rm -f buffer.o
	rm -f client
	rm -f server
