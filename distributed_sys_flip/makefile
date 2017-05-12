CC      = gcc

all: clique 

clique:  thread.o clique-count.o greedyGraphPermute.o distributed_flip.o client_protocol/client.o matrix.o
	$(CC) -pthread -o $@ $^

run: 
	time ./clique

clean:
	rm clique
	rm *.o

