#include "client.h"

#define COORDINATOR1_IP "111.111.1.1"
#define COORDINATOR2_IP "111.111.1.1"
#define COORDINATOR3_IP "111.111.1.1"

#define MAX_PAYLOAD_SIZE 640000

void readCoordinatorMessage(client_struct *client_info) {
	long bytes_read = read(client_info->sockfd, client_info->recvline, MAX_PAYLOAD_SIZE);

	if(bytes_read == 0) {
		connectToCoordinator(client_info);
	}

	printf("%s\n", client_info->recvline);
	
	// clear client_info->recvline string
	memset(client_info->recvline, 0, MAX_PAYLOAD_SIZE);
}

int connectToCoordinator(client_struct *client_info) {
	int i;

	for(i = 0; i < client_info->num_coordinators; i++) {
		printf("Connecting to Coordinator %d at IP %s", 1 + i, client_info->coordinator_ips[i]);

		client_info->serveraddr.sin_family = AF_INET;
		client_info->serveraddr.sin_addr.s_addr = inet_addr(client_info->coordinator_ips[i]);
		client_info->serveraddr.sin_port = htons(client_info->coordinator_ports[i]); 

		if(connect(client_info->sockfd, (struct sockaddr *)&client_info->serveraddr, sizeof(client_info->serveraddr)) < 0) {
			printf("Connection Failed to Coordinator %d\n", i);
			perror("Error");
		} else {
			client_info->known_coordinator = i;
			break;
		}
	}
	if(i == client_info->num_coordinators) {
		printf("Could not connect to any Coordinators!\n");
		perror("ERROR");
		return -1;
	}
	printf("Connected to Coordinator %d at IP %s\n", client_info->known_coordinator, client_info->coordinator_ips[client_info->known_coordinator]);
	exit(1);
}

// thread entry point for client
void *StartClient(void *args) {
	client_struct *client_info;

	client_info = (client_struct*)malloc(sizeof(client_struct));

	client_info->known_coordinator = 0;
	client_info->num_coordinators = 3;
	client_info->coordinator_ips = (char **)malloc(sizeof(char*) * client_info->num_coordinators);
	client_info->coordinator_ips[0] = (char *)malloc(strlen(COORDINATOR1_IP) + 1);
	client_info->coordinator_ips[1] = (char *)malloc(strlen(COORDINATOR2_IP) + 1);
	client_info->coordinator_ips[2] = (char *)malloc(strlen(COORDINATOR3_IP) + 1);
	client_info->coordinator_ports[0] = 400;
	client_info->coordinator_ports[1] = 400;
	client_info->coordinator_ports[2] = 400;
	client_info->recvline = (char*)malloc(sizeof(char) * MAX_PAYLOAD_SIZE);
	bzero(client_info->recvline, MAX_PAYLOAD_SIZE);

	client_info->sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(client_info->sockfd < 0)
	{
		printf("Error creating socket");
		exit(1);
	}

	connectToCoordinator(client_info);
	

	while(1)
		readCoordinatorMessage(client_info);
}