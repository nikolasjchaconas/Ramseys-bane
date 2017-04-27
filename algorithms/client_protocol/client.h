#ifndef CLIENT_H
#define CLIENT_H

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>
#include <netdb.h>
#include <string.h>
#include <fcntl.h>

typedef struct _client_struct {
	int known_coordinator;
	int num_coordinators;
	char **coordinator_ips;
	int *coordinator_ports;
	int sockfd;
	struct sockaddr_in serveraddr;
	char *recvline;
} client_struct;


int getRandomNumber(int bound);

int readCoordinatorMessage(int *out_matrix, int counter_number);
int connectToCoordinator();
int sendCounterExampleToCoordinator(int* matrix, int counter_number, int* out_matrix);
int pollCoordinator(int* out_matrix);
int numDigits(int num);
void createClient();
double getTime();
void setPollInterval(double interval);

extern client_struct *client_info;
extern double last_poll_time;
extern double poll_interval;

#endif