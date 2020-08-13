#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>


#define CMD_LED_D7 _IOW('L',0,unsigned int)
#define CMD_LED_D8 _IOW('L',1,unsigned int)
#define CMD_LED_D9 _IOW('L',2,unsigned int)
#define CMD_LED_D10 _IOW('L',3,unsigned int)

int main(int argc,char **argv)
{
	
	int fd;
	int i;
	int len;

	
	fd = open("/dev/myled",O_RDWR);
	
	if(fd < 0)
	{
		
		perror("open:");
		return -1;
		
	}

	while(1)
	{
		//D7灯的亮灭
		ioctl(fd,CMD_LED_D7,1);
		sleep(1);
		ioctl(fd,CMD_LED_D7,0);

		//D8
		ioctl(fd,CMD_LED_D8,1);
		sleep(1);
		ioctl(fd,CMD_LED_D8,0);

		//D9
		ioctl(fd,CMD_LED_D9,1);
		sleep(1);
		ioctl(fd,CMD_LED_D9,0);

		//D10
		ioctl(fd,CMD_LED_D10,1);
		sleep(1);
		ioctl(fd,CMD_LED_D10,0);
	}
	
	
	//关闭myled的设备
	close(fd);
	
	return 0;
	
	
}