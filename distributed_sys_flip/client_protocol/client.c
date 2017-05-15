#include "client.h"

#define COORDINATOR1_IP "169.231.235.33"
#define COORDINATOR2_IP "169.231.235.124"
#define COORDINATOR3_IP "169.231.235.86"
#define COORDINATOR4_IP "169.231.235.115"
#define COORDINATOR5_IP "169.231.235.97"

#define NUM_COORDINATORS 5
#define COORDINATOR_PORT 5001

double poll_interval_seconds = 180;

int getRandomNumber(int bound) {
	int rand_num;

	rand_num = (double)rand()/RAND_MAX * bound;
	return rand_num;
}

double getTime() {
	struct timeval time;
	if (gettimeofday(&time,NULL)){
	   return 0;
	}
	return (double)time.tv_sec + (double)time.tv_usec * .000001;
}

void setPollInterval(double interval, client_struct *client_info) {
	poll_interval_seconds = interval;
	client_info->last_poll_time = -interval;
}

int readCoordinatorMessage(int counter_number, client_struct *client_info) {
	long bytes_read;
	char *counter_index;
	char *clique_index;
	char *row_index;
	char buffer[1024];
	int i;
	int matrix_size;
	int digits;
	int size;
	int *out_matrix;
	int num;

	num = 1;
	bytes_read = 0;
	out_matrix = client_info->coordinator_return->out_matrix;

	bzero(buffer, 1024);
	bzero(client_info->recvline, LARGEST_MATRIX_SIZE * sizeof(char));

	while(num > 0) {
		num = read(client_info->sockfd, client_info->recvline + bytes_read, LARGEST_MATRIX_SIZE - bytes_read);
		bytes_read += num;
	}

	if(bytes_read <= 0) {
		printf("Connection was lost\n");
		return -1;
	}

	//coordinator_number
	counter_index = strchr(client_info->recvline, ':');
	if(!counter_index) {
		printf("Sujaya you forgot the colon character!!!\n");
		return -1;
	}
	strncpy(buffer, client_info->recvline, counter_index - client_info->recvline);
	client_info->coordinator_return->counter_number = atoi(buffer);
	bzero(buffer, 1024);

	//coordinator_clique
	clique_index = strchr(counter_index, ':');
	if(!clique_index) {
		printf("Sujaya you forgot the colon character!!!\n");
		return -1;
	}
	strncpy(buffer, counter_index, clique_index - counter_index);
	client_info->coordinator_return->clique_count = atoi(buffer);
	bzero(buffer, 1024);

	//coordinator_index
	row_index = strchr(clique_index, ':');
	if(!row_index) {
		printf("Sujaya you forgot the colon character!!!\n");
		return -1;
	}
	strncpy(buffer, clique_index, row_index - clique_index);
	client_info->coordinator_return->index = atoi(buffer);
	bzero(buffer, 1024);

	// if my current counter number is less than the one the coordinator has
	if(counter_number < client_info->coordinator_return->counter_number && out_matrix) {
		matrix_size = client_info->coordinator_return->counter_number * client_info->coordinator_return->counter_number;
		bzero(out_matrix, LARGEST_MATRIX_SIZE * sizeof(int));
		for(i = 0; i < matrix_size; i++) {
			out_matrix[i] = *(client_info->recvline + digits + i + 1) - '0';
		}
	}

	return client_info->coordinator_return->counter_number;
}

int numDigits(int num) {
	int digits;

	if(num == 0) return 1;
	digits = 0;

	while(num > 0) {
		num /= 10;
		digits++;
	}

	return digits;
}

int pollCoordinator(int counter_number, int clique_count, int index, int* matrix, client_struct *client_info) {
	double time_waited;

	time_waited = getTime() - client_info->last_poll_time;

	// printf("time waited is %f and interval is %f and last time is %f\n", time_waited, poll_interval_seconds, last_poll_time);
	if(time_waited > poll_interval_seconds) {
		printf("Polling\n");
		return sendCounterExampleToCoordinator(counter_number, clique_count, index, matrix, client_info);
	} else {
		return 0;
	}
}

int sendCounterExampleToCoordinator(int counter_number, int clique_count, int index, int* matrix, client_struct *client_info) {
	int error;
	int len;
	int ret;
	int size;
	int i;
	int counter_digits;
	int clique_digits;
	int index_digits;
	int offset;

	double time_waited;


	client_info->last_poll_time = getTime();

	error = connectToCoordinator(client_info);

	if(error) {
		close(client_info->sockfd);
		return -1;
	}
	printf("Sending to Coordinator: counter num: %d, clique count: %d, index: %d\n", counter_number, clique_count, index);
	counter_digits = numDigits(counter_number);
	clique_digits = numDigits(clique_count);
	index_digits = numDigits(index);
	offset = counter_digits + clique_digits + index_digits + 3;

	// matrix size + digits in counter example number + colon
	size = (counter_number * counter_number) + offset;
	bzero(client_info->sendline, LARGEST_MATRIX_SIZE * sizeof(char));
	
	// print counter number
	sprintf(client_info->sendline, "%d:", counter_number);
	// print clique count

	sprintf(client_info->sendline + counter_digits + 1, "%d:", clique_count);

	// print index
	sprintf(client_info->sendline + counter_digits + clique_digits + 2, "%d:", index);

	// print matrix
	for(i = offset; i < size; i++) {
		sprintf(client_info->sendline + i, "%d", matrix[i - offset]);
	}

	// printf("writing %d bytes of :\n%s\n", size, client_info->sendline);
	// form the message and send it
	ret = write(client_info->sockfd, client_info->sendline, size);

	if(ret <= 0) {
		fprintf(stderr, "Error: wrote %d bytes to coordinator\n", ret);
	}
	// printf("wrote out %d bytes to server: %s\n", ret, buffer);

	//out_matrix will contain currrent counter example
	ret = readCoordinatorMessage(counter_number, client_info);
	printf("Read from the Coordinator\n");

	if(ret < 0) {
		printf("error\n");
	}

	close(client_info->sockfd);

	return ret;


	// may use this later
		// ret = getsockopt (client_info->socket_fd, SOL_SOCKET, SO_ERROR, &error, &len);

	// if(ret != 0) {
	// 	fprintf(stderr, "error\n");
	// 	connectToCoordinator();
	// }
	// else if(error != 0) {
	// 	fprintf(stderr, "error\n");
	// 	connectToCoordinator();
	// }
}

int connectToCoordinator(client_struct *client_info) {
	int i;
	int coordinator;
	int ret;

	
	client_info->sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(client_info->sockfd < 0)
	{
		perror("Error creating socket\n");
		return 1;
	}

	//try to connect to random coordinator
	coordinator = getRandomNumber(client_info->num_coordinators);

	printf("Connecting to Coordinator %d at IP %s\n", 1 + coordinator, client_info->coordinator_ips[coordinator]);
	
	memset(&client_info->serveraddr, 0, sizeof(client_info->serveraddr));
	client_info->serveraddr.sin_family = AF_INET;
	client_info->serveraddr.sin_addr.s_addr = inet_addr(client_info->coordinator_ips[coordinator]);
	client_info->serveraddr.sin_port = htons(client_info->coordinator_ports[coordinator]); 

	ret = connect(client_info->sockfd, (struct sockaddr *)&client_info->serveraddr, sizeof(client_info->serveraddr)); 

	if (ret == 0) {
		client_info->known_coordinator = coordinator;
	} else {
		printf("Connection Failed to Coordinator %d\n", coordinator + 1);
		perror("ERROR1");
		for(i = 0; i < client_info->num_coordinators; i++) {
			if(i != coordinator) {
				close(client_info->sockfd);
				client_info->sockfd = socket(AF_INET, SOCK_STREAM, 0);
				if(client_info->sockfd < 0)
				{
					perror("Error creating socket\n");
					return 1;
				}
				printf("Connecting to Coordinator %d at IP %s\n", 1 + i, client_info->coordinator_ips[i]);

				memset(&client_info->serveraddr, 0, sizeof(client_info->serveraddr));
				client_info->serveraddr.sin_family = AF_INET;
				client_info->serveraddr.sin_addr.s_addr = inet_addr(client_info->coordinator_ips[i]);
				client_info->serveraddr.sin_port = htons(client_info->coordinator_ports[i]); 

				ret = connect(client_info->sockfd, (struct sockaddr *)&client_info->serveraddr, sizeof(client_info->serveraddr));

				if (ret == 0){
					client_info->known_coordinator = i;
					break;
				} else {
					printf("Connection Failed to Coordinator %d\n", i + 1);
					perror("ERROR2");
				}
			}
		}
	}

	

	if(i == client_info->num_coordinators) {
		return 1;
	}

	printf("Connected to Coordinator %d at IP %s\n", client_info->known_coordinator + 1, client_info->coordinator_ips[client_info->known_coordinator]);
	return 0;
}

void createClient(client_struct *client_info) {
	printf("Creating Client...\n\n");
	client_info->coordinator_return->out_matrix = (int *)malloc(sizeof(int) * LARGEST_MATRIX_SIZE);
	client_info->known_coordinator = 0;
	client_info->last_poll_time = 0;
	client_info->num_coordinators = NUM_COORDINATORS;
	client_info->recvline = (char*)malloc(sizeof(char) * LARGEST_MATRIX_SIZE);
	client_info->sendline = (char*)malloc(sizeof(char) * LARGEST_MATRIX_SIZE);
	client_info->coordinator_ips = (char **)malloc(sizeof(char*) * client_info->num_coordinators);
	client_info->coordinator_ips[0] = (char *)malloc(strlen(COORDINATOR1_IP) + 1);
	client_info->coordinator_ips[1] = (char *)malloc(strlen(COORDINATOR2_IP) + 1);
	client_info->coordinator_ips[2] = (char *)malloc(strlen(COORDINATOR3_IP) + 1);
	client_info->coordinator_ips[3] = (char *)malloc(strlen(COORDINATOR4_IP) + 1);
	client_info->coordinator_ips[4] = (char *)malloc(strlen(COORDINATOR5_IP) + 1);
	strcpy(client_info->coordinator_ips[0], COORDINATOR1_IP);
	strcpy(client_info->coordinator_ips[1], COORDINATOR2_IP);
	strcpy(client_info->coordinator_ips[2], COORDINATOR3_IP);
	strcpy(client_info->coordinator_ips[3], COORDINATOR4_IP);
	strcpy(client_info->coordinator_ips[4], COORDINATOR5_IP);
	client_info->coordinator_ports = (int *)malloc(sizeof(int) * client_info->num_coordinators);
	client_info->coordinator_ports[0] = COORDINATOR_PORT;
	client_info->coordinator_ports[1] = COORDINATOR_PORT;
	client_info->coordinator_ports[2] = COORDINATOR_PORT;
	client_info->coordinator_ports[3] = COORDINATOR_PORT;
	client_info->coordinator_ports[4] = COORDINATOR_PORT;
}
