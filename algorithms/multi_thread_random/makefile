CC      = gcc

all: clique 

clique:  main.o clique-count.o thread.o matrix.o  ../client_protocol/client.o
	$(CC) -lm -pthread -o $@ $^

run: 
	nohup ./clique &

clean:
	rm *.o
	rm clique

