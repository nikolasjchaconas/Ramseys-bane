CC      = gcc
HOST=`hostname`

all: clique 

clique:  main.o clique-count.o thread.o matrix.o  client_protocol/client.o algorithms.o
	$(CC) -pthread -o $@ $^ -lm

run: 
	nohup ./clique > logs/$(HOST).out 2>&1&

clean:
	rm *.o
	rm clique
