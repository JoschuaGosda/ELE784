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
	   int 	ret;
	unsigned char  val ;

	   
		  
	   
	fd = open("/dev/MyModuleNode1", O_RDONLY);
	   if (fd < 0) {
		   printf("Erreur d'ouverture = %d\n", fd);
		   return -1;
	   }

	read(fd, &val, sizeof(val));
		printf("Data read  = %u\n", val);

 	close(fd);
	   printf("\n");


	   return EXIT_SUCCESS;
}
