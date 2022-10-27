#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define DEFAULT_SERIALPORT	"/dev/SerialDev_Node0"

int main() {
	   int 	fd,fd1;
	   char tp = 0;
	   int 	ret;
	   	  
	   
	fd = open("/dev/MyModuleNode0", O_RDONLY);
	   if (fd < 0) {
		   printf("Erreur d'ouverture = %d\n", fd);
		   return -1;
	   }


	   close(fd);
	   printf("\n");


	fd = open("/dev/MyModuleNode0", O_WRONLY);
	   if (fd < 0) {
		   printf("Erreur d'ouverture = %d\n", fd);
		   return -1;
	   }


	   close(fd);
	   printf("\n");


	fd = open("/dev/MyModuleNode0", O_RDWR);
	   if (fd < 0) {
		   printf("Erreur d'ouverture = %d\n", fd);
		   return -1;
	   }





	fd1 = open("/dev/MyModuleNode1", O_RDWR);
	   if (fd1 < 0) {
		   printf("Erreur d'ouverture1 = %d\n", fd1);
		   return -1;
	   }


	   close(fd);
	   printf("\n");
	   close(fd1);
	   printf("\n");


	   return EXIT_SUCCESS;
}
