CC      = gcc

all: clique 

clique:  main.o clique-count.o
	$(CC) -o $@ $^

run: 
	time ./clique

clean:
	rm clique
	rm *.o

