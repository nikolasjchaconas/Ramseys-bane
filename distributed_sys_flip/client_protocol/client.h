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

#define LARGEST_MATRIX_SIZE 1024*1024

extern long double CPU_CLOCK_SPEED;

typedef struct _coordinator_struct {
	int *out_matrix;
	int counter_number;
	int clique_count;
	int index;
} coordinator_struct;

typedef struct _client_struct {
	int known_coordinator;
	int num_coordinators;
	char **coordinator_ips;
	int *coordinator_ports;
	int id;
	int sockfd;
	struct sockaddr_in serveraddr;
	char *recvline;
	char *sendline;
	double last_poll_time;
	double last_cpu_send_time;
	clock_t initial_time;
	coordinator_struct *coordinator_return;
} client_struct;


int getRandomNumber(int bound);

int readCoordinatorMessage(int counter_number, client_struct *client_info);
int connectToCoordinator();
void sendCPUCycles(client_struct *client_info);
int sendCounterExampleToCoordinator(int counter_number, int clique_count, int index, int* matrix, client_struct *client_info);
int pollCoordinator(int counter_number, int clique_count, int index, int* matrix, client_struct *client_info);
int numDigits(int num);
void createClient();
double getTime();
void setPollInterval(double interval, client_struct *client_info);

extern double poll_interval;

#endif