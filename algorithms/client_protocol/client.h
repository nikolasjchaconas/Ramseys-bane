#ifndef CLIENT_H
#define CLIENT_H

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <string.h>

typedef struct _client_struct {
	int known_coordinator;
	int num_coordinators;
	char **coordinator_ips;
	int *coordinator_ports;
	int sockfd;
	struct sockaddr_in serveraddr;
	char *recvline;
} client_struct;

void readCoordinatorMessage(client_struct *client_info);
int connectToCoordinator(client_struct *client_info);

#endif