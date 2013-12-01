#include <stdint.h>
#include "../uc-inc/stdlib.h"
__attribute__ ((noinline)) volatile uint64_t fx(){
	return  256ll;
}
__attribute__ ((noinline)) volatile uint64_t fy(){
	return  128ll;
}
__attribute__ ((noinline)) int main(){
	uint64_t x = fx(), y = fy();
	uint64_t z = x + y;
	return (z == 384);
}