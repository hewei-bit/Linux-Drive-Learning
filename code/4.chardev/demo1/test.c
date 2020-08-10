#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc,char **argv)
{
	
	int fd;
	
	
	fd = open("/dev/myled",O_RDWR);
	
	if(fd < 0)
	{
		
		perror("open:");
		return -1;
		
	}
	
	//睡眠3秒
	sleep(3);
	
	//关闭myled的设备
	close(fd);
	
	return 0;
	
	
}