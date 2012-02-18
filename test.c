/*
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>

int main(void)
{
	int fd;
	//int fd1;
	char s[] = "This is test\n";
	char buffer[80];
	//int size = 0;

	fd = open("/dev/cdata", O_RDWR);
	//fd1 = open("/dev/cdata3", O_RDWR);
	//sleep(10);

	write(fd, s, sizeof(s));
	//size = read(fd, buffer, sizeof(buffer));
	close(fd);

	//sleep(10);
	//close(fd1);

	return 0;
}
