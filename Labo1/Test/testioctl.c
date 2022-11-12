#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "ioctl_config.h"

#define DEFAULT_SERIALPORT "/dev/SerialDev_Node0"

int main() {
    int fd, fd1;
    int tp1;
    unsigned char tp [5] = {1, 2, 3, 4, 5};
    int ret;

    fd = open("/dev/MyModuleNode0", O_WRONLY );
    if (fd < 0) {
        printf("Erreur d'ouverture = %d\n", fd);
        return -1;
    }

    printf("Type new size of buffer \n");
     scanf("%d",&tp1);

    ret = ioctl(fd, SETBUFFERSIZE, tp1);
    //ret = ioctl(fd, GETBUFFERSIZE, NULL);
    printf("Return value of ioctl %d\n", ret);
    //ret = write(fd, &tp, 5);
    //printf("Return value of writing 5 data %d\n", ret);


    /*
    if (ret < 1) {
        printf("Writing not possible, buffer full\n");
    }
    else {
        printf("%u data elements written \n", ret);
    } */

    close(fd);

    return EXIT_SUCCESS;
}
