#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <pthread.h>
#include <errno.h>
#include "communication_p.h"
#include "communication.h"

using namespace std;

void * server_thread(
	void * argp)
{
	struct server_arg * server_arg_p= (struct server_arg *) argp;
	pthread_mutex_t * mutex_table= server_arg_p->mutex_table;
	unsigned long int mutex_table_size= server_arg_p->mutex_table_size;
	unsigned char * bloom_filter= server_arg_p->bloom_filter;
	unsigned long int size= server_arg_p->size;
	struct sockaddr_in socket_address;
	socket_address.sin_addr.s_addr= htonl(INADDR_ANY);
	socket_address.sin_port= htons(server_arg_p->port);
	socket_address.sin_family= AF_INET;
	int sock;
	if( (sock= socket(PF_INET, SOCK_STREAM, 0)) == -1){
		cerr<<"sock creation failed: "<<strerror(errno)<<endl;
		close(sock);
		return 0;
	}
	int optval= 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void *) &optval, sizeof(optval));
	if(bind(sock, (struct sockaddr *) &socket_address, sizeof(socket_address)) == -1){
		cerr<<"coudlnt bind server sock: "<<strerror(errno)<<endl;
		close(sock);
		return 0;
	}
	if(listen(sock, SERVER_BACKLOG_NO) == -1){
		cerr<<"server couldnt listen: "<<strerror(errno)<<endl;
		close(sock);
		return 0;
	}
	int serving_sock;
	bool incoming_answer;
	struct arg_struct arg_struct_1;
	arg_struct_1.size= server_arg_p->size;
	arg_struct_1.k= server_arg_p->k;
	arg_struct_1.seed= server_arg_p->seed;
	cout<<"Server set up and ready to receive connections\n";
	while(true){
		if((serving_sock= accept(sock, NULL, NULL)) == -1){
			cerr<<"faield to accept: "<<strerror(errno)<<endl;
			close(sock);
			return 0;
		}
		if(write(serving_sock, (void *) &arg_struct_1, sizeof(arg_struct_1)) != sizeof(arg_struct_1)){	//send args to client
			cerr<<"couldnt send lsit of arguments to client"<<endl;
			close(serving_sock);
			close(sock);
			return 0;
		}
		cout<<"server: list of arguments sent to requester\n";
		if(read(serving_sock, (void *) &incoming_answer, sizeof(incoming_answer)) != sizeof(incoming_answer)){	//receive answer
			cerr<<"didnt receive legible answer from client\n";
			close(serving_sock);
			close(sock);
			return 0;
		}
		char packet[PACKET_SIZE];
		if(incoming_answer == true){	//if yes, then send bloom filter
			cout<<"server: bloom filter is being copied to requester\n";
			unsigned long int bytes_transferred= 0;
			unsigned int actual_packet_size= PACKET_SIZE;
			for(unsigned int i= 0; i < mutex_table_size; i++){
				pthread_mutex_lock(&mutex_table[i]);
			}
			while(bytes_transferred < size){
				if(bytes_transferred + PACKET_SIZE > size)
					actual_packet_size= size - bytes_transferred;
				memcpy((void *) packet, (void *) &bloom_filter[bytes_transferred], actual_packet_size);
				bytes_transferred+= actual_packet_size;
				if(write(serving_sock, (void *) packet, actual_packet_size) != actual_packet_size){
					cerr<<"server couldnt send part of bloom filter: "<<strerror(errno)<<endl;
					close(sock);
					close(serving_sock);
					return 0;
				}
			}
			for(unsigned int i= 0; i < mutex_table_size; i++){
				pthread_mutex_unlock(&mutex_table[i]);
			}
			cout<<"server: bloom filter copied succesfuly\n";
		}
		close(serving_sock);
	}
}