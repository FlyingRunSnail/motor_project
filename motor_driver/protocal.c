#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "protocal.h"

static int calc_checksum(comm_frame_t *frame, unsigned char *checksum)
{
    if (!frame)
    {
        printf("input param frame invalid.\n");
        return -1;
    }

    if (!checksum)
    {
        printf("input param checksum invalid.\n");
        return -1;
    }

    *checksum = 0;
    *checksum += frame->start;
    *checksum += frame->addr;
    *checksum += frame->command;

    *checksum += (unsigned char)(frame->data & 0xff)
    *checksum += (unsigned char)((frame->data >> 8) & 0xff)
    *checksum += (unsigned char)((frame->data >> 16) & 0xff)
    *checksum += (unsigned char)((frame->data >> 24) & 0xff)
    
    return 0;
}

comm_frame_t * alloc_frame(unsigned char addr, unsigned char command, unsigned int data)
{
    unsigned char checksum;
    int ret;
    comm_frame_t *frame;

    frame = malloc(sizeof(*frame));
    if (!frame)
    {
        printf("malloc failed.\n");
        return 0;
    }

    memset(frame, 0, sizeof(*frame));

    frame->start = FRAME_HEAD;
    frame->addr = addr;
    frame->command = command;
    frame->data = data;

    ret = calc_checksum(frame, &checksum);
    if (ret < 0)
    {
        printf("call calc_checksum failed.\n");
        return 0;
    }

    frame->checksum = checksum;
    
    return frame;
}

comm_frame_t * modify_frame(comm_frame_t *frame, unsigned char addr, unsigned char command, unsigned int data)
{
    int ret;

    if (!frame)
    {
        frame = alloc_frame(addr, command, data);
        return frame;
    }

    frame->addr = (frame->addr != addr) ? addr : frame->addr;
    frame->command = (frame->command != command) ? command : frame->command;
    frame->data = (frame->data != data) ? data : frame->data;

    ret = calc_checksum(frame, &checksum);
    if (ret < 0)
    {
        printf("call calc_checksum failed.\n");
        return 0;
    }

    frame->checksum = checksum;
     
    return frame;
}

void free_frame(comm_frame_t *frame)
{
    if (frame)
    {
        free(frame);
    }

    return ;
}


