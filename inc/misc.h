#ifndef _UC32SIM_MISC_H
#define _UC32SIM_MISC_H
#define static_assert(expr) do {switch (expr) {case 0: case expr: break;} }while(0)

static inline uint32_t min(uint32_t a, uint32_t b){
	return a < b? a: b;
}
static inline uint32_t max(uint32_t a, uint32_t b){
	return a > b? a: b;
}
#endif