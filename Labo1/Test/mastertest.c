#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define DEFAULT_SERIALPORT "/dev/SerialDev_Node0"

int main() {
    int fd, fd1;
    int tp1, portId, userType;
    
    int ret;
	
	printf("HELLO Module PROGRAM\n");
	printf("Select Port 0 or 1\n");
	scanf("%d",&portId);
	printf("Select writer(0) or reader(1)\n");
	scanf("%d",&userType);
	if( portId ==0 && userType==0 ){
		
		printf("Selected Port is  0 as writer\n");
    		fd = open("/dev/MyModuleNode0", O_WRONLY | O_NONBLOCK);
    		if (fd < 0) {
        	printf("Erreur d'ouverture = %d\n", fd);
        	return -1;
    		}
			printf("How many data to write\n");
    	 	scanf("%d",&tp1);
    		 printf("Okay %d data to write\n",tp1);
   		 char tp [tp1];
    		ret = write(fd, &tp,tp1);

    		if (ret < 1) {
     		   printf("Writing not possible, buffer full\n");
   		 }
   		 else {
    	    printf("%u data elements written \n", ret);
   	 	}





	}else if ( portId ==0 && userType==1){
		printf("Selected Port is  0 as reader\n");
		fd = open("/dev/MyModuleNode0", O_RDONLY | O_NONBLOCK);
    		if (fd < 0) {
        	printf("Erreur d'ouverture = %d\n", fd);
        	return -1;}


     		printf("How many data to read\n");
     		scanf("%d",&tp1);
    		 printf("Okay %d data to read\n",tp1);
    		 char tp [tp1];
    		ret = read(fd, &tp, tp1);
    		if (ret < 1) {
      		  printf("Not all data as been read \n");
    		}
    		printf("%u data elements read \n", ret);

    		

	} else if( portId ==1 && userType==0){
		printf("Selected Port is  1 as writer\n");
		fd = open("/dev/MyModuleNode1", O_WRONLY | O_NONBLOCK);
    		if (fd < 0) {
        	printf("Erreur d'ouverture = %d\n", fd);
        	return -1;}


		printf("How many data to write\n");
     scanf("%d",&tp1);
     printf("Okay %d data to write\n",tp1);
    char tp [tp1];
    ret = write(fd, &tp,tp1);

    if (ret < 1) {
        printf("Writing not possible, buffer full\n");
    }
    else {
        printf("%u data elements written \n", ret);
    }



    	} else if( portId ==1 && userType==1){
		printf("Selected Port is  1 as reader\n");
		fd = open("/dev/MyModuleNode1", O_RDONLY | O_NONBLOCK);
    		if (fd < 0) {
        	printf("Erreur d'ouverture = %d\n", fd);
        	return -1;}
    	} else {
	printf("Erreur de choix \n");
		return 0;		
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
