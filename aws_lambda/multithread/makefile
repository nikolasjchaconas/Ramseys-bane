CC      = gcc

all: clique 

clique:  main.o clique-count.o thread.o matrix.o
	$(CC) -pthread -o $@ $^

run: 
	time ./clique

clean:
	rm clique
	rm *.o

