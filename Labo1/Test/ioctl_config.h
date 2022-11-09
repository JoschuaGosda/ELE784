

#include <linux/ioctl.h>

#define IOC_MAGIC 'k'
#define SETBAUDRATE         _IOW(IOC_MAGIC,0,int)
#define SETDATASIZE         _IOW(IOC_MAGIC,1,int)
#define SETPARITY           _IOW(IOC_MAGIC,2,int)
#define GETBUFFERSIZE       _IOR(IOC_MAGIC,3,int)
#define SETBUFFERSIZE       _IOW(IOC_MAGIC,4,int)
#define IOC_MAXNR 5