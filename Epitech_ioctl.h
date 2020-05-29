#ifndef EPITECH_IOCTL_H_
#define EPITECH_IOCTL_H_

#include <asm/ioctl.h>

#define EPITECH_DRV_CLEAR_BUFFER 0
#define EPITECH_DRV_SYNC_BARRIER 1

// ioctl macros
#define QUERY_GET_VARIABLES _IOR('q', 1, query_arg_t *)
#define QUERY_CLR_VARIABLES _IO('q', 2)

typedef struct
{
    int status;
    int dignity;
    int ego;
} query_arg_t;

#endif /* !EPITECH_IOCTL_H_ */