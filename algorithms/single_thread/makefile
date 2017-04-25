CC      = gcc

all: clique 

clique:  main.o clique-count.o ../client_protocol/client.o
	$(CC) -o $@ $^

run: 
	time ./clique

clean:
	rm *.o

