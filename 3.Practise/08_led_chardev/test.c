#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(int argc,char **argv) {
    
    int fd;
    int len;

    char buf[] = "hewei test"; 

    fd = open("/dev/myled",O_RDWR);

    if(fd < 0)
    {
        perror("open");
        return -1;
    }    
    
	//睡眠3秒
	sleep(3);

    len = write(fd,buf,strlen(buf));
	
	printf("len = %d\n",len);
   
    bzero(buf,sizeof(buf));

    len = read(fd,buf,5);

	if(len > 0)
		printf("read buf:%s\n len = %d\n",buf,len);

    //睡眠3秒
	sleep(3); 

    close(fd);

    return 0;
}