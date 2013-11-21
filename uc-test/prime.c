#include "../uc-inc/stdlib.h"
#define MAX 10001
uint32_t primes[MAX], n = 3;
int add_prime(uint32_t num){
	int i;
	for (i = 2; i < n; ++i){
		if (primes[i] * primes[i] > num){
			primes[n++] = num;
			return 1;
		}
		if (num % primes[i] == 0)
			break;
	}
	return 0;
}
int main()
{
    uint32_t i = 0;
    primes[0] = 2;
    primes[1] = 3;
    primes[2] = 5;
    while (n < MAX){
    	while (1){
    		i += 6;
    		if (add_prime(i + 1) + add_prime(i + 5)) break;
    	}
    }
    for (i = 0; i < n; i++)
    	sys_putuint(primes[i]);
    return 0;
}