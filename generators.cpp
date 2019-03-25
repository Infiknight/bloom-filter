#include <cstdlib>
#include <ctime>
#include <cmath>

void construct_random_word(
	char * word, 
	int size,
	unsigned int * seedp)
{
	for(int i= 0; i < size; i++){
		word[i]= 65 + floor( ((double) rand_r(seedp))/ (((double) RAND_MAX) + 1)*25 );
	}
	word[size]= '\0';
}