#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "protocol.h"

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

    *checksum += (unsigned char)(frame->data & 0xff);
    *checksum += (unsigned char)((frame->data >> 8) & 0xff);
    *checksum += (unsigned char)((frame->data >> 16) & 0xff);
    *checksum += (unsigned char)((frame->data >> 24) & 0xff);
    
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
    unsigned char checksum;
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


void dump_frame(comm_frame_t *frame)
{
    if (frame)
    {
        printf("start:       0x%02x\n", frame->start);
        printf("addr:        0x%02x\n", frame->addr);
        printf("command:     0x%02x\n", frame->command);
        printf("data:        0x%08x\n", frame->data);
        printf("checksum:    0x%02x\n", frame->checksum);
	
        printf("\n");
    }
}

int check_frame(comm_frame_t *frame)
{
    int ret;
    unsigned char checksum;

    if (!frame)
    {
        printf("invalid input param frame.\n");
        return -1;
    }

    if (frame->start != FRAME_HEAD)
    {
        printf("frame start field 0x%02x and expected 0x%02x\n", frame->start, FRAME_HEAD);
        return -1;
    }

    ret = calc_checksum(frame, &checksum);   
    if (ret < 0)
    {
        printf("calc checksum failed.\n");
        return -1;
    }

    return 0;
}

void free_frame(comm_frame_t *frame)
{
    if (frame)
    {
        free(frame);
    }

    return ;
}


