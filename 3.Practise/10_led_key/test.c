#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(int argc,char **argv)
{
	
	int fd_led;
	int fd_key;	
	char key_val=0;
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
		len =read(fd_key,&key_val,1);
		if(len > 0)
		{
	//D7 --->GPIOE13, K4按键的电平，对应key_val的bit2	
//D8 --->GPIOC17,K3按键的电平，对应key_val的bit1
// D9 --->GPIOC8,K2按键的电平，对应key_val的bit0
// D10--->GPIOC7,K6按键的电平，对应key_val的bit3	
			if(key_val & 0x01)
			{
				printf("K2 Down\n");
				//点灯
				write(fd_led,d9_on,2);
			}
			else
			{
				//灭灯
				write(fd_led,d9_off,2);				
			}

			if(key_val & 0x02)
			{
				printf("K3 Down\n");
				//点灯
				write(fd_led,d8_on,2);
			}
			else
			{
				//灭灯
				write(fd_led,d8_off,2);				
			}

			if(key_val & 0x04)
			{
				printf("K4 Down\n");
				//点灯
				write(fd_led,d7_on,2);
			}
			else
			{
				//灭灯
				write(fd_led,d7_off,2);				
			}

			if(key_val & 0x08)
			{
				printf("K6 Down\n");
				//点灯
				write(fd_led,d10_on,2);
			}
			else
			{
				//灭灯
				write(fd_led,d10_off,2);				
			}

		}
		
		//500ms
		usleep(500*1000);
		
	}
	
	
	//关闭myled的设备
	close(fd_led);
	
	return 0;
	
	
}