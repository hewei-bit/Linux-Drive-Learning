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
	char buf[64]={0};
	
	
	
	fd = open("/dev/myrc522",O_RDWR);
	
	if(fd < 0)
	{
		
		perror("open:");
		return -1;
		
	}

	while(1)
	{
		
		read(fd,buf,sizeof buf);
		
		sleep(1);

	}
	
	
	//关闭myrc522的设备
	close(fd);
	
	return 0;
	
	
}