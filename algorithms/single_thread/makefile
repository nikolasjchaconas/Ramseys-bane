CC      = gcc

all: clique 

clique:  main.o clique-count.o ../client_protocol/client.o
	$(CC) -o $@ $^

run: 
	nohup ./clique &

clean:
	rm *.o

