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
	int fd_beep;

	char key_val=0;
	int distance;
	int len;
	int res;

	char key_flag=0;
	
	char d7_on[2]={7,1};
	char d7_off[2]={7,0};
	
	char d8_on[2]={8,1};
	char d8_off[2]={8,0};
	
	char d9_on[2]={9,1};
	char d9_off[2]={9,0};
	
	char d10_on[2]={10,1};
	char d10_off[2]={10,0};

	char beep_on[2]={1,1};
	char beep_off[2]={1,0};
	
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

	fd_beep = open("/dev/mybeep",O_RDWR);
	if(fd_key < 0)
	{
		perror("open:");
		return -1;
	}	

	while(1)
	{		

		len =read(fd_key,&key_val,1);
		res = read(fd_sr04,&distance,sizeof distance);

// //D7 --->GPIOE13, K4按键的电平，对应key_val的bit2	
// //D8 --->GPIOC17,K3按键的电平，对应key_val的bit1
// // D9 --->GPIOC8,K2按键的电平，对应key_val的bit0
// // D10--->GPIOC7,K6按键的电平，对应key_val的bit3	

		//读取超声波的测距
	// 	＞45cm：理想安全区，所有LED灯熄灭
    // 35~45cm：非常安全区，亮1盏LED灯
    // 25~35cm：安全区，亮2盏LED灯
    // 15~25cm：警告区，亮3盏LED灯
    // ＜15cm：危险区，亮4盏LED灯
	

		if(res > 0 )
		{
			if(distance>450 && distance<=4000)
			{
				printf("distance:%dmm\n",distance);
				write(fd_led,d7_off,2);
				write(fd_led,d8_off,2);
				write(fd_led,d9_off,2);
				write(fd_led,d10_off,2);
				write(fd_beep,beep_off,2);
			}
			else if(distance>350 && distance<=450)
			{
				printf("distance:%dmm\n",distance);
				write(fd_led,d7_on,2);
				write(fd_led,d8_off,2);
				write(fd_led,d9_off,2);
				write(fd_led,d10_off,2);
				write(fd_beep,beep_off,2);
			}
			else if(distance>250 && distance<=350)
			{
				printf("distance get error %d\n",distance);
				write(fd_led,d7_on,2);
				write(fd_led,d8_on,2);
				write(fd_led,d9_off,2);
				write(fd_led,d10_off,2);
				write(fd_beep,beep_off,2);
			}
			else if(distance>150 && distance<=250)
			{
				printf("distance get error %d\n",distance);
				write(fd_led,d7_on,2);
				write(fd_led,d8_on,2);
				write(fd_led,d9_on,2);
				write(fd_led,d10_off,2);
				write(fd_beep,beep_off,2);

			}
			else if(distance>=20 && distance<=150)
			{
				printf("distance get error %d\n",distance);
				write(fd_led,d7_on,2);
				write(fd_led,d8_on,2);
				write(fd_led,d9_on,2);
				write(fd_led,d10_on,2);
				write(fd_beep,beep_on,2);
		
			}
		}
		//500ms
		usleep(500*1000);
		
	}
	
	//关闭myled的设备
	close(fd_led);
	close(fd_sr04);
	close(fd_key);
	close(fd_beep);
	return 0;
	
}