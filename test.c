/*
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include "cdata_ioctl.h"

int main(void)
{
	int fd;
	int i;
//	int j;

	char pix[4] = {0x00, 0xff, 0x00, 0xff};
		
	fd = open("/dev/cdata", O_RDWR);
	for(i =0;i<256;i++)
		write(fd, pix,4);

	//i = 10000;
	//ioctl(fd, CDATA_CLEAR, &i);
/*
	sleep(3);
	ioctl(fd, CDATA_RED);
	sleep(3);
	ioctl(fd, CDATA_GREEN);
	sleep(3);
	ioctl(fd, CDATA_BLUE);
	sleep(3);
	ioctl(fd, CDATA_BLACK);
*/
/*
	for (j=0;j<10;j++){
		ioctl(fd, CDATA_CLEAR, &i);

		sleep(3);
		ioctl(fd, CDATA_RED);

		sleep(3);
		ioctl(fd, CDATA_GREEN);

		sleep(3);
		ioctl(fd, CDATA_BLUE);

		sleep(3);
	}
*/
	//fd1 = open("/dev/cdata3", O_RDWR);
	//sleep(10);

	//write(fd, s, sizeof(s));
	//size = read(fd, buffer, sizeof(buffer));
	close(fd);

	//sleep(10);
	//close(fd1);

	return 0;
}
