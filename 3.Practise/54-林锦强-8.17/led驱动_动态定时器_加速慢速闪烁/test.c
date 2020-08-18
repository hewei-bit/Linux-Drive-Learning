#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

#define CMD_LED7  _IOW('l', 0, unsigned int)
#define CMD_LED8  _IOW('l', 1, unsigned int)
#define CMD_LED9  _IOW('l', 2, unsigned int)
#define CMD_LED10 _IOW('l', 3, unsigned int)


int main(int argc,char **argv)
{
	
	int fd_led;	
	int i;
	int len;
	
	
	fd_led = open("/dev/myled",O_RDWR);
	if(fd_led < 0)
	{
		
		perror("open:");
		return -1;
		
	}
	

	while(1);
	
	
	//关闭myled的设备
	close(fd_led);

	
	return 0;
	
	
}