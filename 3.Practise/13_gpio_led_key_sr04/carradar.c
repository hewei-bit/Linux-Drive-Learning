#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>


int main(int argc, char const *argv[])
{
	int keyfd,ledfd,hrcsfd,beepfd;

	keyfd = open("/dev/mykey_drv",O_RDWR);
	
	if(keyfd < 0)
	{
		
		printf("keyfd error\n");
		perror("open:");
		return -1;
		
	}	

	beepfd = open("/dev/mybeep_drv",O_RDWR);
	
	if(keyfd < 0)
	{
		printf("beepfd error\n");
		
		perror("open:");
		return -1;
		
	}	

	ledfd = open("/dev/myled_drv",O_RDWR);
	
	if(ledfd < 0)
	{
		printf("ledfd error\n");
		
		perror("open:");
		return -1;
		
	}	

	hrcsfd = open("/dev/myhcsr04_drv",O_RDWR);
	
	if(hrcsfd < 0)
	{
		printf("hrcsfd error\n");
		
		perror("open:");
		return -1;
		
	}	

	char buf1[2] = {'1','0'};
	char buf2[2] = {'1','1'};
	char buf3[2] = {'2','0'};
	char buf4[2] = {'2','1'};
	char buf5[2] = {'3','0'};
	char buf6[2] = {'3','1'};
	char buf7[2] = {'4','0'};
	char buf8[2] = {'4','1'};



	int len = 0;
	char key_val = 0;
	while(1)
	{
		len =read(keyfd,&key_val,1);
		if(len > 0)
		{
			if(key_val & 0x01)
					break;
		}

		usleep(50000);//降低CPu占用

	}


	printf("radar star\n");
	write(ledfd,buf2,2);
	usleep(100000);
	write(ledfd,buf1,2);
	usleep(100000);
	write(ledfd,buf4,2);
	usleep(100000);
	write(ledfd,buf3,2);
	usleep(100000);
	write(ledfd,buf6,2);
	usleep(100000);
	write(ledfd,buf5,2);
	usleep(100000);
	write(ledfd,buf8,2);
	usleep(100000);
	write(ledfd,buf7,2);
	usleep(100000);

	


	int distance;
	while(1)
	{
		len = read(hrcsfd,&distance,sizeof distance);
		if(len > 0)
		{
			if(distance>=20 && distance<=4000)
			{
				printf("distance:%dmm\n",distance);
				if(distance < 150)
				{
						write(ledfd,buf2,2);
						write(ledfd,buf4,2);
						write(ledfd,buf6,2);
						write(ledfd,buf8,2);
						write(beepfd,"1",1);
				}
				else if(distance < 250)
				{
						// write(ledfd,buf2,2);
						write(ledfd,buf4,2);
						write(ledfd,buf6,2);
						write(ledfd,buf8,2);
				}
				else if(distance < 350)
				{
						// write(ledfd,buf4,2);
						write(ledfd,buf6,2);
						write(ledfd,buf8,2);
				}
				else if(distance <450)
				{
						// write(ledfd,buf6,2);
						write(ledfd,buf8,2);
				}
				else
				{
						write(ledfd,buf1,2);
						write(ledfd,buf3,2);
						write(ledfd,buf5,2);
						write(ledfd,buf7,2);
				}			
			}
			else
			{
				printf("distance get error %d\n",distance);
						write(ledfd,buf1,2);
						write(ledfd,buf3,2);
						write(ledfd,buf5,2);
						write(ledfd,buf7,2);
				
			}
		}



		len =read(keyfd,&key_val,1);
		if(len > 0)
		{
			if(key_val & 0x02)
					break;
		}

						usleep(50000);//降低CPu占用
						write(ledfd,buf1,2);
						write(ledfd,buf3,2);
						write(ledfd,buf5,2);
						write(ledfd,buf7,2);
						write(beepfd,"0",1);
						usleep(50000);
	}

	printf("radar stop\n");
	close(ledfd);
	close(keyfd);
	close(hrcsfd);







	return 0;
}