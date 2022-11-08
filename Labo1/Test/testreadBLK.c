#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define DEFAULT_SERIALPORT "/dev/SerialDev_Node1"

int main() {
    int fd, fd1;
   // char tp [8] = {0, 0};
    int tp1;
    int ret =0;
    unsigned char val;

    fd = open("/dev/MyModuleNode0", O_RDONLY);
    // fd1 = open("/dev/MyModuleNode1", O_WRONLY | O_NONBLOCK);
    if (fd < 0) {
        printf("Erreur d'ouverture = %d\n", fd);
        return -1;
    }
     printf("How many data to read\n");
     scanf("%d",&tp1);
     printf("Okay %d data to read\n",tp1);
     char tp [tp1];
    ret = read(fd, &tp, tp1);
    if (ret < 1) {
        printf("Not all data as been read \n");
    }
    printf("%u data elements read \n", ret);

    close(fd);

    return EXIT_SUCCESS;
}
