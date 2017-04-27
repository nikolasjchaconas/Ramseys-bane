#include "client.h"

#define COORDINATOR1_IP "169.231.235.33"
#define COORDINATOR2_IP "169.231.235.124"
#define COORDINATOR3_IP "169.231.235.86"

#define MAX_PAYLOAD_SIZE (1024 * 1024)

client_struct *client_info = NULL;

double poll_interval_seconds = 60;
double last_poll_time = 0;

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

void setPollInterval(double interval) {
	poll_interval_seconds = interval;
	last_poll_time = -interval;
}

int readCoordinatorMessage(int *out_matrix, int counter_number) {
	long bytes_read;
	int coordinator_number;
	char *index;
	char buffer[1024];
	int i;
	int matrix_size;
	int digits;
	int size;

	bzero(buffer, 1024);

	bzero(client_info->recvline, MAX_PAYLOAD_SIZE * sizeof(char));

	bytes_read = read(client_info->sockfd, client_info->recvline, MAX_PAYLOAD_SIZE);

	if(bytes_read == 0) {
		printf("Connection was lost\n");
		return -1;
	}

	index = strchr(client_info->recvline, ':');

	if(!index) {
		printf("Sujaya you forgot the colon character!!!\n");
		return -1;
	}

	

	strncpy(buffer, client_info->recvline, index - client_info->recvline);

	coordinator_number = atoi(buffer);
	digits = numDigits(coordinator_number);
	size = coordinator_number * coordinator_number + digits + 1;

	while(bytes_read < size) {
		bytes_read += read(client_info->sockfd, client_info->recvline + bytes_read, MAX_PAYLOAD_SIZE - bytes_read);
	}

	// if my current counter number is less than the one the coordinator has
	if(counter_number < coordinator_number) {
		matrix_size = coordinator_number * coordinator_number;
		out_matrix = (int *)malloc(sizeof(int) * matrix_size);
		for(i = 0; i < matrix_size; i++) {
			out_matrix[i] = *(client_info->recvline + digits + i + 1) - '0';
		}
	}

	return coordinator_number;
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

int pollCoordinator(int* matrix, int counter_number, int* out_matrix) {
	double time_waited;

	time_waited = getTime() - last_poll_time;

	// printf("time waited is %f and interval is %f and last time is %f\n", time_waited, poll_interval_seconds, last_poll_time);
	if(time_waited > poll_interval_seconds) {
		printf("Polling\n");
		return sendCounterExampleToCoordinator(matrix, counter_number, out_matrix);
	} else {
		return counter_number;
	}
}

int sendCounterExampleToCoordinator(int* matrix, int counter_number, int* out_matrix) {
	int error;
	int len;
	int ret;
	char *buffer;
	int size;
	int i;
	int digits;
	double time_waited;


	last_poll_time = getTime();

	if(!client_info) {
		createClient();
	}

	error = connectToCoordinator();

	if(error) {
		printf("Could not connect to a coordinator\n");
		return -1;
	}

	digits = numDigits(counter_number);

	// matrix size + digits in counter example number + colon
	size = (counter_number * counter_number) + digits + 1;

	// create buffer for sending to coordinator
	buffer = (char *)malloc(sizeof(char) * size);
	bzero(buffer, size * sizeof(char));

	sprintf(buffer, "%d:", counter_number);
	for(i = digits + 1; i < size; i++) {
		sprintf(buffer + i, "%d", matrix[i - digits - 1]);
	}

	// form the message and send it
	ret = write(client_info->sockfd, buffer, size);
	if(ret <= 0) {
		fprintf(stderr, "Error: wrote 0 bytes to coordinator\n");
	}

	// printf("wrote out %d bytes to server: %s\n", ret, buffer);

	//out_matrix will contain currrent counter example
	ret = readCoordinatorMessage(out_matrix, counter_number);
	printf("Read from the coordinator\n");

	if(ret < 0) {
		printf("error\n");
	}
	free(buffer);
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

int connectToCoordinator() {
	int i;
	int coordinator;
	int ret;

	
	client_info->sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(client_info->sockfd < 0)
	{
		printf("Error creating socket");
		return 1;
	}

	//try to connect to random coordinator
	coordinator = getRandomNumber(client_info->num_coordinators);

	printf("Connecting to Coordinator %d at IP %s\n", 1 + coordinator, client_info->coordinator_ips[coordinator]);

	client_info->serveraddr.sin_family = AF_INET;
	client_info->serveraddr.sin_addr.s_addr = inet_addr(client_info->coordinator_ips[coordinator]);
	client_info->serveraddr.sin_port = htons(client_info->coordinator_ports[coordinator]); 

	ret = connect(client_info->sockfd, (struct sockaddr *)&client_info->serveraddr, sizeof(client_info->serveraddr)); 

	if (ret == 0) {
		printf("Connection Succeeded.\n");
		client_info->known_coordinator = coordinator;
	} else {
		printf("Connection Failed to Coordinator %d\n", coordinator + 1);
		perror("CONNECTION ERROR");
		for(i = 0; i < client_info->num_coordinators; i++) {
			if(i != coordinator) {
				printf("Connecting to Coordinator %d at IP %s\n", 1 + i, client_info->coordinator_ips[i]);

				client_info->serveraddr.sin_family = AF_INET;
				client_info->serveraddr.sin_addr.s_addr = inet_addr(client_info->coordinator_ips[i]);
				client_info->serveraddr.sin_port = htons(client_info->coordinator_ports[i]); 

				ret = connect(client_info->sockfd, (struct sockaddr *)&client_info->serveraddr, sizeof(client_info->serveraddr));

				if (ret == 0){
					printf("Connection Succeeded.\n");
					client_info->known_coordinator = i;
					break;
				} else {
					printf("Connection Failed to Coordinator %d\n", i + 1);
					perror("CONNECTION ERROR");
				}
			}
		}
	}

	

	if(i == client_info->num_coordinators) {
		printf("Could not connect to any Coordinators!\n");
		perror("ERROR");
		return 1;
	}

	printf("Connected to Coordinator %d at IP %s\n", client_info->known_coordinator + 1, client_info->coordinator_ips[client_info->known_coordinator]);
	return 0;
}

void createClient() {
	client_info = (client_struct*)malloc(sizeof(client_struct));

	client_info->known_coordinator = 0;
	client_info->num_coordinators = 3;
	client_info->coordinator_ips = (char **)malloc(sizeof(char*) * client_info->num_coordinators);
	client_info->coordinator_ips[0] = (char *)malloc(strlen(COORDINATOR1_IP) + 1);
	client_info->coordinator_ips[1] = (char *)malloc(strlen(COORDINATOR2_IP) + 1);
	client_info->coordinator_ips[2] = (char *)malloc(strlen(COORDINATOR3_IP) + 1);
	strcpy(client_info->coordinator_ips[0], COORDINATOR1_IP);
	strcpy(client_info->coordinator_ips[1], COORDINATOR2_IP);
	strcpy(client_info->coordinator_ips[2], COORDINATOR3_IP);
	client_info->coordinator_ports[0] = 5001;
	client_info->coordinator_ports[1] = 5001;
	client_info->coordinator_ports[2] = 5001;
}