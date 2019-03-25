/* 
 * File:   main.cpp
 * Author: Infiknight
 *
 * Created on February 28, 2015, 4:37 PM
 */

#include <string>
#include <iostream>
#include <climits>
#include <ctime>
#include <cstdlib>
#include "list.h"
#include "hash.h"
#include "oracle.h"
#include <cctype>
#include <cstring>
#include "searcher_thread.h"
#include <sys/time.h>
#include "communication.h"

using namespace std;

#define MAX_MUTEXES_NO 20

long int time_diff_usec(
	struct timeval start,
	struct timeval end)
{
	return (end.tv_sec*1000000 + end.tv_usec) - (start.tv_sec*1000000 + start.tv_usec);
}

/*
 * 
 */
int main(int argc, char** argv) {
	struct timeval time_1, time_2;
	int seed= 34;
	initSeed(seed);
	unsigned long int size;
	int	port,
		L,
		N;
	char * end;
	char * address;

	bool h_set_already= false,
		k_set_already= false;
	int no_optional_args= 1;
	int optional_id= 6,
		optional_arg= 7;
	char * logfile_name;
	int num= 3;
	switch(argc){	//command line arguments
	case 10:
		optional_id+= 2;
		optional_arg+= 2;
		no_optional_args++;
	case 8:
		for(int i= 0; i < no_optional_args; i++){
			if( (strcmp("-h", argv[optional_id]) == 0) && (h_set_already == false) ){
				h_set_already= true;
				address= argv[optional_arg];
			}
			else if( (strcmp("-k", argv[optional_id]) == 0) && (k_set_already == false) ){
				k_set_already= true;
				num= strtol(argv[optional_arg], &end, 10);
				if(*end != '\0'){
					cerr<<"invalid NUM"<<endl;
					return 1;
				}
			}
			else{
				cerr<<"duplicate optional args"<<endl;
				return 1;
			}
			optional_id-= 2;
			optional_arg-= 2;
		}
	case 6:
		logfile_name= argv[5];
		port= strtol(argv[4], &end, 10);
		if(*end != '\0'){
			cerr<<"invalid PORT"<<endl;
			return 1;
		}
		L= strtol(argv[3], &end, 10);
		if(*end != '\0'){
			cerr<<"invalid L"<<endl;
			return 1;
		}
		N= strtol(argv[2], &end, 10);
		if(*end != '\0'){
			cerr<<"invalid N"<<endl;
			return 1;
		}
		size= strtoul(argv[1], &end, 10);
		if(*end != '\0'){
			cout<<"invalid size\n";
			return 1;
		}
		break;
	default:
		cerr<<"wrong number of arguments"<<endl;
		return 1;
	}
	
	unsigned long int section_multiplier= 1;
	unsigned long int mutex_table_size= (size * CHAR_BIT) / 512;
	if(mutex_table_size > MAX_MUTEXES_NO){	//choose a mutex for each bloom filter section
		mutex_table_size= MAX_MUTEXES_NO;
		section_multiplier= (size * CHAR_BIT) / (512 * MAX_MUTEXES_NO);
	}
	if((size * CHAR_BIT) % (section_multiplier * 512) != 0)
		section_multiplier++;
	pthread_mutex_t mutex_table[mutex_table_size];
	for(unsigned long int i= 0; i < mutex_table_size; i++){
		pthread_mutex_init( &(mutex_table[i]), NULL);
	}
	unsigned char * bloom_filter= (unsigned char *) malloc(size * sizeof(char));
	if(bloom_filter == 0){
		cerr<<"lalala"<<endl;
		return 1;
	}
	memset( (void *) bloom_filter, 0, size * sizeof(char));
	if(h_set_already){
		if(initialize_bloom(address, bloom_filter, size, num, seed) != 0){	//copy bloom filter from remote host
			cerr<<"bloom initialization failed\n";
			return 1;
		}
	}
	pthread_t thread_no[N];
	struct searcher_thread_arg arg[N];
	unsigned int seed_for_word= time(NULL);
	gettimeofday(&time_1, NULL);
	for(int i= 0; i < N; i++){	//worker threads
		arg[i].logfile_name= logfile_name;
		arg[i].size= size;
		arg[i].L= L;
		arg[i].num= num;
		arg[i].seed= seed_for_word + i;
		arg[i].bloom_filter= bloom_filter;
		arg[i].mutex_table= mutex_table;
		arg[i].mutex_table_size= mutex_table_size;
		arg[i].mutex_table_section_multiplier= section_multiplier;
		pthread_create( &thread_no[i], NULL, searcher_thread, (void *) &(arg[i]));
	}
	pthread_t server_t;
	struct server_arg server_arg_1;
	server_arg_1.mutex_table= mutex_table;
	server_arg_1.mutex_table_size= mutex_table_size;
	server_arg_1.bloom_filter= bloom_filter;
	server_arg_1.port= port;
	server_arg_1.size= size;
	server_arg_1.k= num;
	server_arg_1.seed= seed;
	pthread_create(&server_t, NULL, server_thread, (void*) &server_arg_1);	//server thread
	for(int i= 0; i < N; i++){
		pthread_join(thread_no[i], NULL);
	}
	gettimeofday(&time_2, NULL);
	cout<<"exec time: "<<time_diff_usec(time_1, time_2)<<endl;
	free(bloom_filter);
}

