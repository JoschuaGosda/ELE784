#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define DEFAULT_SERIALPORT "/dev/SerialDev_Node0"

int main()
{
	int fd, fd1, fd2;
	char tp = 0;
	int ret;

	printf("Test 5 should fail! \n\n");

	// open port 0 twice, one time READ, one time WRITE
	fd2 = open("/dev/MyModuleNode0", O_RDONLY);
	if (fd < 0)
	{
		printf("Erreur d'ouverture1 = %d\n", fd);
		return -1;
	}
	else {
		printf("Test1 success\n");
	}

	fd = open("/dev/MyModuleNode0", O_WRONLY);
	if (fd < 0)
	{
		printf("Erreur d'ouverture2 = %d\n", fd);
		return -1;
	}
	else
	{
		printf("Test2 success\n");
	}

	close(fd);
	close(fd2);


	// open port 0 in RDWR
	fd = open("/dev/MyModuleNode0", O_RDWR);
	if (fd < 0)
	{
		printf("Erreur d'ouverture3 = %d\n", fd);
		return -1;
	}
	else
	{
		printf("Test3 success\n");
	}

	close(fd);

	// open port 1 in RDWR
	fd1 = open("/dev/MyModuleNode1", O_RDWR);
	if (fd1 < 0)
	{
		printf("Erreur d'ouverture4 = %d\n", fd1);
		return -1;
	}
	else
	{
		printf("Test4 success\n");
	}

	// open port 1 in WR
	fd = open("/dev/MyModuleNode1", O_WRONLY);
	if (fd < 0)
	{
		printf("Erreur d'ouverture5 = %d\n", fd);
		return -1;
	}

	close(fd);
	close(fd1);

	return EXIT_SUCCESS;
}
