#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>


#define CMD_LED_D7		_IOW('L',0,unsigned int)

int main(int argc,char **argv)
{

	int fd;
	fd = open("/dev/myled",O_RDWR);
	
	if(fd < 0)
	{
		
		perror("open:");
		return -1;
		
	}

	while(1)
	{
		ioctl(fd,CMD_LED_D7,1);//亮灯
		sleep(1);
		
		ioctl(fd,CMD_LED_D7,0);//灭灯
		sleep(1);		
		

	}
	
	
	//关闭myled的设备
	close(fd);
	
	return 0;
	
	
}