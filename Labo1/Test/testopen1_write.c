#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define DEFAULT_SERIALPORT	"/dev/SerialDev_Node0"

int main() {
	   int 	fd,fd1;
	  unsigned char tp = 8;
	   int 	ret;

	   
		  
	   
	fd = open("/dev/MyModuleNode0", O_WRONLY);
	   if (fd < 0) {
		   printf("Erreur d'ouverture = %d\n", fd);
		   return -1;
	   }
	write(fd, &tp,sizeof(tp));



	   close(fd);
	   printf("\n");
	  

	   return EXIT_SUCCESS;
}
