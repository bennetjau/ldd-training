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
	char pix_green[4] = {0x00, 0x00, 0xff, 0x00};

	i = 10000;
	char *fb;

	fd = open("/dev/cdata", O_RDWR);

	fb = mmap(0, 1024, PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
	//sleep(25);

	while(1)
		write(fb, pix_green,4);

	close(fd);
	return 0;

/*
	char pix_blue[4] = {0x00, 0x00, 0x00, 0xff};
	char pix_green[4] = {0x00, 0x00, 0xff, 0x00};

	pid_t pid = fork();

		
	fd = open("/dev/cdata", O_RDWR);
	//for(i =0;i<320*240;i++)


	if(pid == 0){
		//parent process
		while(1)
			write(fd, pix_green,4);
	}else{
		//child process
		while(1)
			write(fd, pix_blue,4);
	}

	//i = 320*240;
	//ioctl(fd, CDATA_CLEAR, &i);
*/
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
	//close(fd);

	//sleep(10);
	//close(fd1);

	//return 0;
}
