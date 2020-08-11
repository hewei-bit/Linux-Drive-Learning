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
	
	char wbuf[]="teacher.wen";
	
	char rbuf[16]={0};
	
	
	fd = open("/dev/myled",O_RDWR);
	
	if(fd < 0)
	{
		
		perror("open:");
		return -1;
		
	}
	
	//睡眠3秒
	sleep(3);
	
	len = write(fd,wbuf,strlen(wbuf));
	
	printf("len = %d\n",len);
	
	//睡眠3秒
	sleep(3);

	len = read(fd,rbuf,sizeof rbuf);
	
	printf("read:");
	for(i=0; i<len; i++)
	{
		printf("%d ",rbuf[i]);
		
	}
	printf("\n");
	printf("len = %d\n",len);
	
	//睡眠3秒
	sleep(3);	
	
	
	//关闭myled的设备
	close(fd);
	
	return 0;
	
	
}