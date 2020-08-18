#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>


#define CMD_LED_D7		_IOW('L',0,unsigned int)
#define CMD_LED_D8		_IOW('L',1,unsigned int)
#define CMD_LED_D9		_IOW('L',2,unsigned int)
#define CMD_LED_D10		_IOW('L',3,unsigned int)

#define CMD_KEY_GET		_IOR('K',0,unsigned int *)


int main(int argc,char **argv)
{

	int fd_led;
	int fd_key;
	int flag1=0;
	int flag2=0;
	int flag3=0;
	int flag4=0;
	unsigned int key_val=0;
	
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
		key_val=0;
		ioctl(fd_key,CMD_KEY_GET,&key_val);
		
		if(key_val & 0x01)
		{
			if(flag1==0)
			{
				ioctl(fd_led,CMD_LED_D7,1);//点灯
				flag1=!flag1;
			}
			else
			{
				ioctl(fd_led,CMD_LED_D7,0);//灭灯
				flag1=!flag1;
			}
		}
		if(key_val & 0x02)
		{
			if(flag2==0)
			{
				ioctl(fd_led,CMD_LED_D8,1);//灭灯
				flag2=!flag2;
			}
			else
			{
				ioctl(fd_led,CMD_LED_D8,0);//灭灯
				flag2=!flag2;
			}
		}
		if(key_val & 0x04)
		{
			if(flag3==0)
			{
				ioctl(fd_led,CMD_LED_D9,1);//灭灯
				flag3=!flag3;
			}
			else
			{
				ioctl(fd_led,CMD_LED_D9,0);//灭灯
				flag3=!flag3;
			}
		}
		if(key_val & 0x08)
		{
			if(flag4==0)
			{
				ioctl(fd_led,CMD_LED_D10,1);//灭灯
				flag4=!flag4;
			}
			else
			{
				ioctl(fd_led,CMD_LED_D10,0);//灭灯
				flag4=!flag4;
			}
		}		
		//usleep(10*1000);
		
		printf("main is running...\n");

	}
	
	
	//关闭myled的设备
	close(fd_led);
	
	//关闭mykey的设备
	close(fd_key);	
	
	return 0;
	
	
}