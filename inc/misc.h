#ifndef _UC32SIM_MISC_H
#define _UC32SIM_MISC_H
#define static_assert(expr) do {switch ((int)expr) {case 0: case (int)expr: break;} }while(0)
#endif