CC      = gcc
HOST=`hostname`

all: clique

clique:  thread.o clique-count.o greedyGraphPermute.o distributed_flip.o client_protocol/client.o matrix.o getcpuclockspeed.o cliquer.o graph.o reorder.o
	$(CC) -pthread -o $@ $^ -lm

run:
	nohup ./clique $(threads) > logs/$(HOST).out 2>&1&

clean:
	rm clique
	rm *.o
