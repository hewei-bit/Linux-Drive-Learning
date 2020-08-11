#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(int argc,char **argv)
{
	

	int fd_sr04;	
	char key_val=0;
	int distance;
	int len;
	

	
	fd_sr04 = open("/dev/mysr04",O_RDWR);
	
	if(fd_sr04 < 0)
	{
		
		perror("open:");
		return -1;
		
	}	

	while(1)
	{
		//读取超声波的测距
		len = read(fd_sr04,&distance,sizeof distance);
		
		if(len > 0)
		{
			if(distance>=20 && distance<=4000)
			{
				printf("distance:%dmm\n",distance);
			}
			else
			{
				printf("distance get error %d\n",distance);
				
			}
		}
		
		
		//睡眠1秒
		sleep(1);
	}
	
	
	//关闭mysr04的设备
	close(fd_sr04);
	
	return 0;
	
	
}