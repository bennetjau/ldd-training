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
	fd = open("/dev/cdata", 0_RDWR);
	close(fd);
	

	return 0;
}
