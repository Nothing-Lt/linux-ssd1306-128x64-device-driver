#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>

int main(int argc, char** argv)
{
    int i = 0;
    int fd = 0;
    char buf[100];

    fd = open("/dev/SSD1306",O_RDWR);
    if(-1 == fd){
        printf("Failed in openning file\n");
        return 0;
    }

    while(1){
        sprintf(buf,"%d",i++);
        lseek(fd, 0, SEEK_SET);
        write(fd,buf,strlen(buf));
        sleep(1);
    }

    printf("it is ready\n");

    printf("Releasing spi\r\n");
    close(fd);

    return 0;
}