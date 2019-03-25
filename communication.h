#define SERVER_BACKLOG_NO 10

struct server_arg{
	pthread_mutex_t * mutex_table;
	unsigned long int mutex_table_size;
	unsigned char * bloom_filter;
	int port;
	unsigned long int size;
	int k;
	int seed;
};

void * server_thread(
	void * argp);

int initialize_bloom(
	char * address,
	unsigned char * bloom_filter,
	unsigned long int size,
	int k,
	int seed);