#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define DEFAULT_SERIALPORT "/dev/SerialDev_Node1"

int main() {
    int fd, fd1;
    char tp [2] = {0, 0};
    char tp1;
    int ret;
    unsigned char val;

    fd = open("/dev/MyModuleNode0", O_RDONLY);
    // fd1 = open("/dev/MyModuleNode1", O_WRONLY | O_NONBLOCK);
    if (fd < 0) {
        printf("Erreur d'ouverture = %d\n", fd);
        return -1;
    }

    ret = read(fd, &tp, 2);
    if (ret < 1) {
        printf("No data available, buffer empty or error\n");
    }
    else {
        printf("%u data elements read \n", ret);
    }

    close(fd);

    return EXIT_SUCCESS;
}
