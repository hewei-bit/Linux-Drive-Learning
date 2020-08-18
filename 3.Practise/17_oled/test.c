#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
// #include <stdio.h>
// #include <sys/types.h>
// #include <sys/stat.h>
#include <sys/mman.h>
// #include <fcntl.h>
// #include <unistd.h>
#include <errno.h>
#include <linux/input.h>
#include <linux/ioctl.h>

#define CMD_LED_D7		_IOW('L',0,unsigned int)
#define CMD_LED_D8		_IOW('L',1,unsigned int)
#define CMD_LED_D9		_IOW('L',2,unsigned int)
#define CMD_LED_D10		_IOW('L',3,unsigned int)

#define CMD_KEY_GET _IOR('K',0,unsigned int *)


void touch(int *x, int *y)
{
	int fd = open("/dev/input/event0", O_RDWR);
	if(fd<0)
	{
		perror("open");
		return ;
	}
	
	struct input_event buf;
	int tmp_x=0, tmp_y=0;
	while(1)
	{
		read(fd, &buf, sizeof(buf));
	
		if(buf.type==EV_ABS && buf.code==ABS_X)
		{
			tmp_x = buf.value;
		}	
		
		if(buf.type==EV_ABS && buf.code==ABS_Y)
		{
			tmp_y = buf.value;
		}
		
		if(tmp_x!=0 && tmp_y!=0)
		{
			*x = tmp_x;
			*y = tmp_y;
			break;
		}
	}
	close(fd);
}

int main(int argc,char **argv)
{
	
	int fd;
	fd = open("/dev/myoled_drv",O_RDWR);
	if(fd < 0)
	{
		
		perror("open:");
		return -1;
		
	}

	write(fd,"1",sizeof("1"));

	
	sleep(3);
	//关闭myled的设备
	close(fd);
	
	return 0;
	
	
}