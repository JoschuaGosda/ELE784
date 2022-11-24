#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stropts.h>
#include <string.h>
#include <linux/ioctl.h>
#include <stdint.h>

#include "dht_data.h"
#include "../module/ioctl_cmds.h"
#include "../module/usbvideo.h"

int main(int argc, char *argv[]) {
	int camera_stream;
	int camera_control;
	struct usb_request ioctl_request;
	uint8_t   data[26] = { 0x1b, 0x17, 0xda, 0xce, 0x9f, 0xfc, 0x10, 0x2e, 0x00, \
						   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
						   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t  *format_index = &data[2];
	uint8_t  *frame_index  = &data[3];
	int ret;
	int i;

	FILE *foutput;
	unsigned char *inBuffer;
	unsigned char *finalBuf;
	int32_t		   bytes;


	// Take from arg the camera device
	if (argc > 1) {
	  camera_stream = open(argv[1], O_RDONLY);
	} else {
	  camera_stream = open("/dev/camera_stream", O_RDONLY);
	}

	if (argc > 2) {
	  camera_control = open(argv[1], O_RDONLY);
	} else {
	  camera_control = open("/dev/camera_control", O_RDONLY);
	}

	// Etape # 1 :	Get default config.
	ioctl_request.data_size = 26;
	ioctl_request.request   = GET_DEF;
	ioctl_request.value     = VS_PROBE_CONTROL;
	ioctl_request.index     = 0;
	ioctl_request.timeout   = 5000;
	ioctl_request.data      = data;

	ret = ioctl(camera_stream, IOCTL_GET, &ioctl_request);
	
	printf("IOCTL_GET (GET_DEF - VS_PROBE_CONTROL) ret = %d\n", ret);
	if (ret < 0)
		goto fermer;
	printf("[ %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8]);
	printf("  %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", data[9], data[10], data[11], data[12], data[13], data[14], data[15], data[16], data[17]);
	printf("  %02x %02x %02x %02x %02x %02x %02x %02x ]\n", data[18], data[19], data[20], data[21], data[22], data[23], data[24], data[25]);
	
	// Etape # 2 :	Set config for image size
	ioctl_request.data_size = 26;
	ioctl_request.request   = SET_CUR;
	ioctl_request.value     = VS_PROBE_CONTROL;
	ioctl_request.index     = 0;
	ioctl_request.timeout   = 5000;
	ioctl_request.data      = data;
	*format_index = 2;
	*frame_index  = 15;
	// frame_index :
	// 	1 =>	640x480	(par défaut)
	// 	2 =>	160x90
	// 	3 =>	160x120
	// 	4 =>	176x144
	// 	5 =>	320x180
	// 	6 =>	320x240
	// 	7 =>	352x288
	// 	8 =>	432x240
	// 	9 =>	640x368
	// 10 =>	800x448
	// 11 =>	800x600
	// 12 =>	864x488
	// 13 =>	960x720
	// 14 =>	1024x576
	// 15 =>	1280x728
	// 16 =>	1600x896 (PAS FONCTIONNEL)
	// 17 =>	1920x1088 (PAS FONCTIONNEL)

	ret = ioctl(camera_stream, IOCTL_SET, &ioctl_request);
	
	printf("IOCTL_SET (SET_CUR - VS_PROBE_CONTROL) ret = %d\n", ret);
	if (ret < 0)
		goto fermer;
	
	// Etape # 3 :	Get current config (required after SET_CUR (Étape # 2))
	ioctl_request.data_size = 26;
	ioctl_request.request   = GET_CUR;
	ioctl_request.value     = VS_PROBE_CONTROL;
	ioctl_request.index     = 0;
	ioctl_request.timeout   = 5000;
	ioctl_request.data      = data;

	ret = ioctl(camera_stream, IOCTL_GET, &ioctl_request);
	
	printf("IOCTL_GET (GET_CUR - VS_PROBE_CONTROL) ret = %d\n", ret);
	if (ret < 0)
		goto fermer;
	printf("[ %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8]);
	printf("  %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", data[9], data[10], data[11], data[12], data[13], data[14], data[15], data[16], data[17]);
	printf("  %02x %02x %02x %02x %02x %02x %02x %02x ]\n", data[18], data[19], data[20], data[21], data[22], data[23], data[24], data[25]);
	
	// Etape # 4 :	Commit current config (required to complete new config)
	ioctl_request.data_size = 26;
	ioctl_request.request   = SET_CUR;
	ioctl_request.value     = VS_COMMIT_CONTROL;
	ioctl_request.index     = 0;
	ioctl_request.timeout   = 5000;
	ioctl_request.data      = data;

	ret = ioctl(camera_stream, IOCTL_SET, &ioctl_request);
	
	printf("IOCTL_SET (SET_CUR - VS_COMMIT_CONTROL) ret = %d\n", ret);
	if (ret < 0)
		goto fermer;

	// Etape # 5 :	Test camera Pan and Tilt
	{	int16_t Data[2];
	
		Data[0] = -1000; // Pan
		Data[1] =  1500; // Tilt
	
		ret = ioctl(camera_control, IOCTL_PANTILT_RELATIVE, Data);
		if (ret < 0)
			goto fermer;
		sleep(2);
	
		Data[0] =  1500; // Pan
		Data[1] = -1000;  // Tilt
	
		ret = ioctl(camera_control, IOCTL_PANTILT_RELATIVE, Data);
		if (ret < 0)
			goto fermer;
		sleep(2);
	}
	
	// Etape # 6 :	Test camera Pan and Tilt Reset
	{	int16_t Data = 0x03;

		ret = ioctl(camera_stream, IOCTL_PANTILT_RESET, &Data);
		if (ret < 0)
			goto fermer;
		sleep(2);
	}

	foutput = fopen("./fichier.jpg", "wb");
	if (foutput == NULL) {
		return EXIT_FAILURE;
	}

	// Etape # 7 : Start video streaming
	inBuffer = malloc((640*480)* sizeof(unsigned char));
	if (inBuffer == NULL) {
		printf("inBuffer malloc Error\n");
	}
	finalBuf = malloc((640*480)* sizeof(unsigned char));
	if (finalBuf == NULL) {
		printf("finalBuf malloc Error\n");
	}

	ret = ioctl(camera_stream, IOCTL_STREAMON);
	printf("IOCTL_STREAMON : ret = %d\n", ret);
	if (ret < 0)
		goto nettoyer;

	// Etape # 8 : Read one image
	bytes = read(camera_stream, inBuffer, (640*480*4));
	if (bytes <= 0) {
		fprintf(stderr, "unable to read image\n");
		return EXIT_FAILURE;
	}
	printf("Num Bytes read = %u\n", bytes);

	// Etape # 9 : Stop video streaming
	ret = ioctl(camera_stream, IOCTL_STREAMOFF);
	printf("IOCTL_STREAMOFF : ret = %d\n", ret);
	if (ret < 0)
		goto nettoyer;

	// Etape # 10 : Prepare and save image to file
	memcpy(finalBuf, inBuffer, HEADERFRAME1);
	memcpy(finalBuf + HEADERFRAME1, dht_data, DHT_SIZE);
	memcpy(finalBuf + HEADERFRAME1 + DHT_SIZE, inBuffer + HEADERFRAME1, (bytes - HEADERFRAME1));
	fwrite(finalBuf, bytes + DHT_SIZE, 1, foutput);
	fclose(foutput);

nettoyer:
	free(inBuffer);
	free(finalBuf);

fermer:
	close(camera_stream);
	close(camera_control);

	return EXIT_SUCCESS;
}

