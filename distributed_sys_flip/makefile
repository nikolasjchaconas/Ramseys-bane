CC      = gcc

all: clique 

clique:  thread.o clique-count.o greedyGraphPermute.o distributed_flip.o client_protocol/client.o matrix.o
	$(CC) -lm -pthread -o $@ $^

run: 
	./clique

clean:
	rm clique
	rm *.o

