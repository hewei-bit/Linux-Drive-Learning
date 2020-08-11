#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(int argc,char **argv)
{
	
	int fd;
	int i;
	int len;
	
	char d7_on[2]={7,1};
	char d7_off[2]={7,0};
	
	char d8_on[2]={8,1};
	char d8_off[2]={8,0};
	
	char d9_on[2]={9,1};
	char d9_off[2]={9,0};
	
	char d10_on[2]={10,1};
	char d10_off[2]={10,0};
	
	fd = open("/dev/myled",O_RDWR);
	
	if(fd < 0)
	{
		
		perror("open:");
		return -1;
		
	}

	while(1)
	{
		//D7灯的亮灭
		write(fd,d7_on,2);
		sleep(1);
		write(fd,d7_off,2);
		
		//D8
		write(fd,d8_on,2);
		sleep(1);
		write(fd,d8_off,2);
		
		//D9
		write(fd,d9_on,2);
		sleep(1);
		write(fd,d9_off,2);

		//D10
		write(fd,d10_on,2);
		sleep(1);
		write(fd,d10_off,2);
		
		//D9
		write(fd,d9_on,2);
		sleep(1);
		write(fd,d9_off,2);

		//D8
		write(fd,d8_on,2);
		sleep(1);
		write(fd,d8_off,2);

	}
	
	
	//关闭myled的设备
	close(fd);
	
	return 0;
	
	
}