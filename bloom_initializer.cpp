#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <errno.h>
#include <cstring>
#include <cstdlib>
#include "communication_p.h"

using namespace std;

int initialize_bloom(
	char * address,
	unsigned char * bloom_filter,
	unsigned long int size,
	int k,
	int seed)
{
	pid_t id= getpid();
	char * addr_text= strtok(address, ":");
	char * port_str= strtok(NULL, ":");
	struct sockaddr_in socket_address;
	inet_pton(AF_INET, addr_text, &socket_address.sin_addr.s_addr);
	int port= atoi(port_str);
	socket_address.sin_port= htons(port);
	socket_address.sin_family= AF_INET;
	int sock;
	if( (sock= socket(PF_INET, SOCK_STREAM, 0)) == -1){
		cerr<<id<<": sock creation failed: "<<strerror(errno)<<endl;
		close(sock);
		return 1;
	}
	if(connect(sock, (struct sockaddr *) &socket_address, sizeof(socket_address)) == -1){
		cerr<<id<<": couldn't establish connection to server: "<<strerror(errno)<<endl;
		close(sock);
		return 1;
	}
	cout<<id<<": connection established with server\n";
	struct arg_struct arg_struct_1;
	if(read(sock, (void*) &arg_struct_1, sizeof(arg_struct_1)) != sizeof(arg_struct_1)){	//recv args from server to compare w local args
		cerr<<id<<": server faield to deliver all of the arguments\n";
		close(sock);
		return 1;
	}
	bool answer;
	char packet[PACKET_SIZE];
	if((k == arg_struct_1.k) && (size == arg_struct_1.size) && (seed == arg_struct_1.seed)){
		cout<<id<<": arguments match, bloom filter is being copied from remote host\n";
		answer= true;
		write(sock, (void *) &answer, sizeof(answer));	//tell the server to send the bloom filter
		unsigned long int bytes_transferred= 0;
		unsigned long int previous_size= 0;
		while(bytes_transferred < size){	//receive bloom filter in chunks
			bytes_transferred+= read(sock,(void *) packet, PACKET_SIZE);
			memcpy((void*) &bloom_filter[previous_size], (void *) packet, bytes_transferred - previous_size);
			previous_size= bytes_transferred;
		}
		cout<<id<<": bloom filter download completed successfully\n";
		close(sock);
		return 0;
	}
	else{
		cerr<<id<<": arguments differ\n";
		answer= false;
		write(sock, (void *) &answer, sizeof(answer));	//tell the server to NOT send the bloom filter
		close(sock);
		return 1;
	}
}