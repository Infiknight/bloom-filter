#ifndef SEARCHER_THREAD_H
#define SEARCHER_THREAD_H

#include <pthread.h>

void * searcher_thread(
	void * argp);
	
struct searcher_thread_arg{
	char * logfile_name;
	unsigned long int size;
	int	L;
	int	num;
	unsigned int seed;
	unsigned char * bloom_filter;
	pthread_mutex_t * mutex_table;
	unsigned long int mutex_table_size;
	unsigned long int mutex_table_section_multiplier;
};

#define SECTION_MULTIPLIER 1
//extern pthread_mutex_t mutex_table[];

#endif