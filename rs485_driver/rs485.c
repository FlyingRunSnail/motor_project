#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

#include "rs485.h"
#include "serial.h"


#define SEND_BUF_LEN (8)
#define RCV_BUF_LEN (8)

static unsigned char rs485_send_buf[SEND_BUF_LEN] = {0};
static unsigned char rs485_rcv_buf[RCV_BUF_LEN] = {0};

static pthread_mutex_t rs485_read_lock;
static pthread_mutex_t rs485_write_lock;

int rs485_init(const char *dev)
{
    int fd = open_serial(dev, 9600, 8, 'N', 1); 
    if (fd < 0)
    {
        printf("open serial %s failed.\n", dev);
        return -1;
    }

    pthread_mutex_init(&rs485_read_lock, NULL);
    pthread_mutex_init(&rs485_write_lock, NULL);

    return 0;
}

static void fill_send_buf(comm_frame_t *frame)
{
    rs485_send_buf[0] = frame->start;
    rs485_send_buf[1] = frame->addr;
    rs485_send_buf[2] = frame->command;
    
    rs485_send_buf[3] = (unsigned char)(frame->data & 0xff);
    rs485_send_buf[4] = (unsigned char)((frame->data >> 8) & 0xff);
    rs485_send_buf[5] = (unsigned char)((frame->data >> 16) & 0xff);
    rs485_send_buf[6] = (unsigned char)((frame->data >> 24) & 0xff);

    rs485_send_buf[7] = frame->checksum;
}

int send_frame(comm_frame_t *frame)
{
    int ret;
    int fd = get_fd();
    if (fd < 0) 
    {
        printf("get invalid fd.\n");
        return -1;
    }

    if (!frame)
    {
        printf("invalid input param frame.\n");
        return -1;
    }

    pthread_mutex_lock(&rs485_write_lock);

    fill_send_buf(frame);

    ret = write(fd, rs485_send_buf, SEND_BUF_LEN);
    if ((ret < 0) || (ret != SEND_BUF_LEN))
    {
        pthread_mutex_unlock(&rs485_write_lock);
        printf("send frame failed.\n");
        tcflush(fd, TCOFLUSH); 
        return -1;
    }

    pthread_mutex_unlock(&rs485_write_lock);

    return ret;
}

int rcv_frame(comm_frame_t *frame)
{
    int length = 0;
    int size = 0;
    int retval;
    int fd = -1;
    fd_set fs_read;
    struct timeval tv;

    fd = get_fd();
    if (fd < 0)
    {
        printf("invalid fd.\n");
        return -1;
    }

    if (!frame)
    {
        printf("invalid input param frame.\n");
        return -1;
    }

    FD_ZERO(&fs_read);
    FD_SET(fd, &fs_read);
    tv.tv_sec  = 5;
    tv.tv_usec = 0;
    retval = select(fd + 1, &fs_read, NULL, NULL, &tv);
    if (retval == -1)
    {
        perror("select()");
        return -1;
    }
    else if (retval)
    {
        printf("Data is available now.\n");
        /* FD_ISSET(0, &rfds) will be true. */
    }
    else
    {
        printf("No data within five seconds.\n");
        return -1;
    }

    if (FD_ISSET(fd, &fs_read))
    {
        pthread_mutex_lock(&rs485_read_lock);

	memset(rs485_rcv_buf, 0, RCV_BUF_LEN);

        do
        {
            size = read(fd, rs485_rcv_buf + length, RCV_BUF_LEN - length);
            if (size == 0)
            {
               usleep(4000);
               size = read(fd, rs485_rcv_buf + length, RCV_BUF_LEN - length);
            }

            length += size;

        }while(size > 0);

        if (length < sizeof(*frame))
        {
	    pthread_mutex_unlock(&rs485_read_lock);
            printf("length: %d is not equal %d\n", length, (int)sizeof(comm_frame_t));
            return -1;
        }

        frame->start = rs485_rcv_buf[0];
        frame->addr = rs485_rcv_buf[1];
        frame->command = rs485_rcv_buf[2];            
        frame->data = ((rs485_rcv_buf[6] << 24) | \
                       (rs485_rcv_buf[5] << 16) | \
                       (rs485_rcv_buf[4] << 8) | \
                        rs485_rcv_buf[3]);

        frame->checksum = rs485_rcv_buf[7];
	pthread_mutex_unlock(&rs485_read_lock);

        if (check_frame(frame) != 0)
        {
            printf("check frame failed.\n");
            return -1;
        }
        return 0;
    }

    printf("poll failed.\n");
    return -1;
}


void rs485_exit(void)
{
    close_serial();

    pthread_mutex_destroy(&rs485_read_lock);
    pthread_mutex_destroy(&rs485_write_lock);

    return ;
}

