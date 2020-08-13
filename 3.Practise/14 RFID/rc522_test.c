#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main(void)
{
	printf("test\n");
	
	int fd = 0;
	fd = open("/dev/myrc522",O_RDWR);
	if(fd < 0)
	{
		perror("open err");
		exit(0);
	}
	
	char buf[21] = {0};
	char  rc522_card_num[10] = {0};
	int i=0;
	while(1)
	{
		bzero(buf,21);
		if(read(fd, &buf, 21) == 0)
		{
			//前5个字节为卡号
			sprintf(rc522_card_num,"%02X%02X%02X%02X%02X",buf[0],
														buf[1],
														buf[2],
														buf[3],					
														buf[4]);		
			printf("卡号为：%s\n",rc522_card_num);
			
			for(i=5; i<21; i++)
			{
				printf("%02X ",buf[i]);
			}
			printf("\n");
		}
		sleep(1);
	}
	
	close(fd);
	return 0;
}