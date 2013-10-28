#include "stdlib.h"

__attribute__ ((noinline, naked)) void _start(){
	int status = main();
	sys_exit(status);
}

int fprintln_hex(int fd, uint value){
	static char buf[16];
	int i = 0;
	do{
		int h = value % 16;
		value /= 16;
		buf[i++] = (char)(h < 10?(h+'0'):(h-10+'a'));
	} while (value);
	int j;
	char tmp;
	for (j = 0; j < i/2; j++){
		tmp = buf[j];
		buf[j] = buf[i-j-1];
		buf[i-j-1] = tmp;
	}
	buf[i++] = '\0';
	return sys_write(fd, buf, i);
}