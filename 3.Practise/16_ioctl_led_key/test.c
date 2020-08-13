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

#define CMD_KEY_GET _IOR('K',0,unsigned int *)


int main(int argc,char **argv)
{
	
	int fd_led;
	int fd_key;

	unsigned int key_val;
	
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
		
		ioctl(fd_key,CMD_KEY_GET,&key_val);
		
		if(key_val & 0x01)
			ioctl(fd_led,CMD_LED_D7,0);//灭灯
		else
			ioctl(fd_led,CMD_LED_D7,1);//亮灯
			
		if(key_val & 0x02)
			ioctl(fd_led,CMD_LED_D8,0);//灭灯
		else
			ioctl(fd_led,CMD_LED_D8,1);//亮灯
		
		if(key_val & 0x04)
			ioctl(fd_led,CMD_LED_D9,0);//灭灯
		else
			ioctl(fd_led,CMD_LED_D9,1);//亮灯	

		if(key_val & 0x08)
			ioctl(fd_led,CMD_LED_D10,0);//灭灯
		else
			ioctl(fd_led,CMD_LED_D10,1);//亮灯	

		
		usleep(10*1000);

	}
	
	
	//关闭myled的设备
	close(fd_led);
	close(fd_key);
	
	return 0;
	
	
}