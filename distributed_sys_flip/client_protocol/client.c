#include "client.h"

#define CYCLES_COORDINATOR "169.231.235.58"
#define CYCLES_COORDINATOR_PORT 5002

#define COORDINATOR_PORT 5001

double poll_interval_seconds = 180;
double send_cpu_interval_seconds = 300;

int dns_to_ip(char *hostname , char* ip)
{
	struct hostent *host_struct;
	struct in_addr **addr_list;
	int i;
	host_struct = gethostbyname(hostname);

	if (host_struct == NULL) 
	{
		// get the host info
		perror("gethostname:");
		return -1;
	}
 
	addr_list = (struct in_addr **) host_struct->h_addr_list;
	 
	strcpy(ip , inet_ntoa(*addr_list[0]) );
	return 1;
}

void sendCPUCycles(client_struct *client_info) {
	double time_waited;
	clock_t new_time;
	clock_t time_passed;
	long double cycles_to_send;
	int ret;
	int size;

	time_waited = getTime() - client_info->last_cpu_send_time;

	if(client_info->id == 0 && time_waited > send_cpu_interval_seconds) {
		new_time = clock();
		time_passed = new_time - client_info->initial_time;
		cycles_to_send = ((long double)time_passed / CLOCKS_PER_SEC ) * CPU_CLOCK_SPEED;

		
		client_info->sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if(client_info->sockfd < 0)
		{
			perror("Error creating socket\n");
			return;
		}

		printf("\nSending CPU cycles to Coordinator at IP %s\n", CYCLES_COORDINATOR);
		
		memset(&client_info->serveraddr, 0, sizeof(client_info->serveraddr));
		client_info->serveraddr.sin_family = AF_INET;
		client_info->serveraddr.sin_addr.s_addr = inet_addr(CYCLES_COORDINATOR);
		client_info->serveraddr.sin_port = htons(CYCLES_COORDINATOR_PORT); 

		ret = connect(client_info->sockfd, (struct sockaddr *)&client_info->serveraddr, sizeof(client_info->serveraddr)); 

		if(ret) {
			close(client_info->sockfd);
			return;
		}
		printf("Sending CPU Cycles of %Lf from time passed of %Lf\n", cycles_to_send, (long double)time_passed / CLOCKS_PER_SEC);

		bzero(client_info->sendline, LARGEST_MATRIX_SIZE);
		size = sprintf(client_info->sendline, "%Lf", cycles_to_send);

		// form the message and send it
		ret = write(client_info->sockfd, client_info->sendline, size);

		if(ret <= 0) {
			fprintf(stderr, "Error: wrote %d bytes to Load Balancer\n", ret);
		}

		printf("Sent Cycles!\n\n");

		close(client_info->sockfd);


		client_info->initial_time = new_time;
		client_info->last_cpu_send_time = getTime();
	}
}

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
	int counter;

	digits = 3;
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
	counter = counter_index - client_info->recvline;
	strncpy(buffer, client_info->recvline, counter);
	client_info->coordinator_return->counter_number = atoi(buffer);
	digits += counter;
	bzero(buffer, 1024);

	//coordinator_clique
	counter_index++;
	clique_index = strchr(counter_index, ':');
	if(!clique_index) {
		printf("Sujaya you forgot the colon character!!!\n");
		return -1;
	}
	counter = clique_index - counter_index;
	strncpy(buffer, counter_index, counter);
	client_info->coordinator_return->clique_count = atoi(buffer);
	digits += counter;
	bzero(buffer, 1024);

	//coordinator_index
	clique_index++;
	row_index = strchr(clique_index, ':');
	if(!row_index) {
		printf("Sujaya you forgot the colon character!!!\n");
		return -1;
	}
	counter = row_index - clique_index;
	strncpy(buffer, clique_index, counter);
	client_info->coordinator_return->index = atoi(buffer);
	digits += counter;
	bzero(buffer, 1024);

	// printf("RECEIVED:\n%s", client_info->recvline);
	// if my current counter number is less than the one the coordinator has
	matrix_size = client_info->coordinator_return->counter_number * client_info->coordinator_return->counter_number;
	bzero(out_matrix, LARGEST_MATRIX_SIZE * sizeof(int));

	for(i = 0; i < matrix_size; i++) {
		out_matrix[i] = *(client_info->recvline + digits + i) - '0';
	}
	// printf("received LINE: %s\n\n", client_info->recvline);

	printf("Received From Load Balancer: counter num: %d, clique count: %d, index: %d\n", client_info->coordinator_return->counter_number, client_info->coordinator_return->clique_count, client_info->coordinator_return->index);

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
	printf("Sending to Load Balancer: counter num: %d, clique count: %d, index: %d\n", counter_number, clique_count, index);
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
		fprintf(stderr, "Error: wrote %d bytes to Load Balancer\n", ret);
	}
	// printf("wrote out %d bytes to server: %s\n", ret, buffer);

	//out_matrix will contain currrent counter example
	ret = readCoordinatorMessage(counter_number, client_info);

	if(ret < 0) {
		printf("error\n");
	}

	close(client_info->sockfd);

	return ret;
}

int connectToCoordinator(client_struct *client_info) {
	int i;
	int ret;
	
	dns_to_ip("CoordinatorLoadBalancer-000988402024.lb.cloud.aristotle.ucsb.edu", client_info->load_balance_ip);


	client_info->sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(client_info->sockfd < 0)
	{
		perror("Error creating socket\n");
		return 1;
	}

	printf("Connecting to Load Balancer at IP %s\n", client_info->load_balance_ip);
	
	memset(&client_info->serveraddr, 0, sizeof(client_info->serveraddr));
	client_info->serveraddr.sin_family = AF_INET;
	client_info->serveraddr.sin_addr.s_addr = inet_addr(client_info->load_balance_ip);
	client_info->serveraddr.sin_port = htons(COORDINATOR_PORT); 

	ret = connect(client_info->sockfd, (struct sockaddr *)&client_info->serveraddr, sizeof(client_info->serveraddr)); 

	if (ret != 0) {
		close(client_info->sockfd);
		printf("Connection Failed to Load Balancer at IP %s\n", client_info->load_balance_ip);
		perror("ERROR1");
		return 1;
	}

	printf("Connected to Load Balancer at IP %s\n", client_info->load_balance_ip);
	return 0;
}

void createClient(client_struct *client_info) {
	printf("Creating Client...\n\n");
	client_info->coordinator_return->out_matrix = (int *)malloc(sizeof(int) * LARGEST_MATRIX_SIZE);
	client_info->last_poll_time = 0;
	client_info->last_cpu_send_time = 0;
	client_info->initial_time = clock();
	client_info->recvline = (char*)malloc(sizeof(char) * LARGEST_MATRIX_SIZE);
	client_info->sendline = (char*)malloc(sizeof(char) * LARGEST_MATRIX_SIZE);
	bzero(client_info->load_balance_ip, 0);
}
