#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(int argc,char **argv)
{
	int fd_sr04;	
	int fd_led;
	int fd_key;	
	char key_val=0;
	int i;
	int distance;
	int len;
	
	char d7_on[2]={7,1};
	char d7_off[2]={7,0};
	
	char d8_on[2]={8,1};
	char d8_off[2]={8,0};
	
	char d9_on[2]={9,1};
	char d9_off[2]={9,0};
	
	char d10_on[2]={10,1};
	char d10_off[2]={10,0};

	fd_sr04 = open("/dev/mySR04",O_RDWR);
	if(fd_sr04 < 0)
	{
		perror("open:");
		return -1;
	}
	
	fd_led = open("/dev/myled",O_RDWR);
	if(fd_led < 0)
	{
		perror("open:");
		return -1;
	}
	
	fd_key = open("/dev/mykey",O_RDWR);
	if(fd_key < 0)
	{
		perror("open:");
		return -1;
	}	

	while(1)
	{		
		// len =read(fd_key,&key_val,1);
		// if(len > 0)
		// {
		// 	if(key_val & 0x01)
		// 	{
		// 		printf("K2 Down\n");
		// 		//点灯
		// 		write(fd_led,d7_on,2);
		// 	}
		// 	else
		// 	{
		// 		//灭灯
		// 		write(fd_led,d7_off,2);				
		// 	}
		// }

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
		
		
		//500ms
		usleep(500*1000);
		

	}
	
	
	//关闭myled的设备
	close(fd_sr04);
	
	return 0;
	
	
}