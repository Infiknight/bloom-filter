#include <string>
#include <iostream>
#include <fstream>
#include <climits>
#include <ctime>
#include <cstdlib>
#include "list.h"
#include "hash.h"
#include "oracle.h"
#include <cctype>
#include <cstring>
#include <string>
#include "searcher_thread.h"
#include "generators.h"

#define SIZE_OF_WORD 6

using namespace std;

static bool word_found= false;

static pthread_mutex_t outfile_mut= PTHREAD_MUTEX_INITIALIZER;

void print_results(
	unsigned long int tried_words_no,
	unsigned long int already_in,
	char * logfile_name,
	pthread_t tid)
{
	pthread_mutex_lock(&outfile_mut);
	ofstream out_file;
	out_file.open(logfile_name, ofstream::app);
	out_file<<"thread "<<tid<<": "<<"tried "<<tried_words_no<<" words.\n";
	out_file<<"thread "<<tid<<": "<<"alread_in/tried words ratio was "<<(double) already_in / (double) tried_words_no<<endl;
	out_file<<"----------------------------------------------------------------\n";
	out_file.close();
	pthread_mutex_unlock(&outfile_mut);
}

void * searcher_thread(
	void * argp)
{
	pthread_t tid= pthread_self();
	struct searcher_thread_arg * arg_ptr= (searcher_thread_arg *) argp;
	char * logfile_name= arg_ptr->logfile_name;
	unsigned long int size= arg_ptr->size;
	int L= arg_ptr->L;
	int num= arg_ptr->num;
	unsigned int * seedp= &(arg_ptr->seed);
	unsigned char * bloom_filter= arg_ptr->bloom_filter;
	pthread_mutex_t * mutex_table= arg_ptr->mutex_table;
	unsigned long int section_multiplier= arg_ptr->mutex_table_section_multiplier;
	unsigned long int tried_words_no= 0,
		already_in_bloom_words_no= 0;
	unsigned char bloom_cell, hash_cell;
	List<string1> overflow_list;
	string1 word;
	const char ** oracle_words= NULL;
	const char ** oracle_memory= NULL;
	bool word_checked;
	unsigned long int section_no;
	uint64_t hash_val;
	char current_word[500];
	initAlloc(malloc);
	for(int j= 0; j < L; j++){
		construct_random_word(current_word, SIZE_OF_WORD, seedp);
		free(oracle_memory);
		oracle_memory= NULL;
		oracle_memory= oracle(current_word);
		oracle_words= oracle_memory;
		do{
			if(oracle_words == NULL){
				print_results(tried_words_no, already_in_bloom_words_no, logfile_name, tid);
				cout<<"word found: "<<current_word<<endl;
				word_found= true;
				return 0;
			}
			while(*oracle_words != NULL){
				word= string1(*oracle_words);
				free((void*) *oracle_words);
				oracle_words++;
				word_checked= true;
				for(int i= 0; i < num; i++){
					hash_val= hash_by(i, word.c_str());
					hash_val%= size*CHAR_BIT;	//truncate to fit BF's size
					bloom_cell= bloom_filter[hash_val/CHAR_BIT];	//retrieve cell to compare
					hash_cell= 1;
					hash_cell<<=(hash_val % CHAR_BIT);	//construct comparing cell
					if((bloom_cell & hash_cell) == 0){	//word has not been tried yet at the oracle
						//calc mutex num to lock
						section_no= (hash_val / (section_multiplier * 512));
						//lock mutex
						pthread_mutex_lock( &(mutex_table[section_no]) );
						bloom_cell= bloom_filter[hash_val/CHAR_BIT];	//retrieve cell to compare
						if((bloom_cell & hash_cell) == 0){
							word_checked= false;
							bloom_filter[hash_val/CHAR_BIT]= (bloom_cell | hash_cell);	//update cell of BF
						}
						//unlock mutex
						pthread_mutex_unlock( &(mutex_table[section_no]) );
					}
				}
				if(word_checked == false)
					overflow_list.push_back(word);	//add it to words to check
				else
					already_in_bloom_words_no++;	//bench counter
				tried_words_no++;
			}
			free(oracle_memory);
			oracle_memory= NULL;
			if(overflow_list.empty() == false){	//if we r in the BF phase and still got words to check
				if(word_found == true){
					print_results(tried_words_no, already_in_bloom_words_no, logfile_name, tid);
					return 0;
				}
				strcpy(current_word, overflow_list.get_back().c_str());
				oracle_memory= oracle(current_word);
				oracle_words= oracle_memory;
				overflow_list.pop_back();
			}
			else{	//BF phase and the OF list is out of words
				if(word_found == true){
					print_results(tried_words_no, already_in_bloom_words_no, logfile_name, tid);
					return 0;
				}
				break;
			}
		}while(true);
	}
	print_results(tried_words_no, already_in_bloom_words_no, logfile_name, tid);
	return 0;
}