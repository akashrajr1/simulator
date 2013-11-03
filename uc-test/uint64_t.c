#include <stdint.h>
volatile uint64_t fx(){
	return  256ll;
}
volatile uint64_t fy(){
	return  128ll;
}
int main(){
	volatile uint64_t x = fx(), y = fy();
	volatile uint64_t z = x + y;
	return (z == 384);
}