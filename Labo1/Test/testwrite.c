#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define DEFAULT_SERIALPORT "/dev/SerialDev_Node0"

int main() {
    int fd, fd1;
    unsigned char tp [2] = {1, 2};
    int ret;

    fd = open("/dev/MyModuleNode0", O_WRONLY );
    if (fd < 0) {
        printf("Erreur d'ouverture = %d\n", fd);
        return -1;
    }

    ret = write(fd, &tp, 2);

    if (ret < 1) {
        printf("Writing not possible, buffer full\n");
    }
    else {
        printf("%u data elements written \n", ret);
    }

    close(fd);

    return EXIT_SUCCESS;
}
