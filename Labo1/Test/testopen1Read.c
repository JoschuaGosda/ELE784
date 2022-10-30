#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define DEFAULT_SERIALPORT	"/dev/SerialDev_Node1"

int main() {
	   int 	fd,fd1;
	   char tp = 0;
	   char tp1;
	   int 	ret;
	unsigned char  val ;

	   
		  
	   
	fd = open("/dev/MyModuleNode0", O_RDONLY);
	//fd1 = open("/dev/MyModuleNode1", O_WRONLY | O_NONBLOCK);
	   if (fd < 0) {
		   printf("Erreur d'ouverture = %d\n", fd);
		   return -1;
	   }
	scanf("%c",&tp1);
	read(fd, &val, 1);
		printf("Data read  = %u\n", val);

 	close(fd);
	   printf("\n");


	   return EXIT_SUCCESS;
}
