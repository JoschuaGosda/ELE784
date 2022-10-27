#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define DEFAULT_SERIALPORT "/dev/SerialDev_Node0"

int main()
{
    int fd;
    char tp = 0;
    int ret;
    char text[8] = "mytest!!";
    char buffer[8];

    fd = open("/dev/MyModuleNode0", O_RDWR);
    if (fd < 0)
    {
        printf("Erreur d'ouverture = %d\n", fd);
        return -1;
    }

    write(fd, &text, 8);

    read(fd, &buffer, 8);

    printf("FILE: Data from Buffer:  %s\n", buffer);

    close(fd);
    printf("\n");
}
