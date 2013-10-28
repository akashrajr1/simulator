//#include<stdint.h>
//#include<sys/syscall.h>
//#include<errno.h>
//#include<fcntl.h>
//uint32_t syscall()
#include "../uc-inc/stdlib.h"
const char *hello = "Hello World!\n";
int main()
{
	//int rc = syscall(SYS_open, "/home/wky/sample.c", O_RDONLY);
	//char buf[256];
	//int fd = _open("/home/wky/sys.c", 0);
	// asm volatile("jepriv #1");
	//int cnt = _read(fd, buf, 256);
	int fd = sys_open("file", O_WRONLY | O_TRUNC);
	char byte = 'h';
	sys_write(fd, hello, sizeof(hello));
	fprintln_hex(fd, 0x19951004);
	sys_lseek(fd, 0, SEEK_SET);
	sys_write(fd, &byte, 1);
	sys_close(fd);
	return 0;
}
