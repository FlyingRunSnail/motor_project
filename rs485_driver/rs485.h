#ifndef __RS485_HEADER__
#define __RS485_HEADER__

#include "protocol.h"

int rs485_init(const char *dev);
void rs485_exit(void);

int send_frame(comm_frame_t *frame);
int rcv_frame(comm_frame_t *frame);

#endif
