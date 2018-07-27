#include "motor_driver.h"
#include "protocol.h"

int motor_init(void)
{
    return 0;
}

int motor_start(motor_id_t motor, rotate_direct_t dir, int nsteps)
{
    int ret;
    comm_frame_t *frame;

    // 设定转动方向
    frame = alloc_frame(0xff, eCMD_0x64, dir);
    if (!frame)
    {
        printf("call alloc_frame failed.\n");
        return -1;
    }

    ret = send_frame(frame);
    if (ret < 0)
    {
        printf("send frame failed.\n");
        return -1;
    }

    ret = rev_frame(frame);
    if (ret < 0)
    {
        printf("rcv frame failed.\n");
        return -1;
    }

    // 设置转动给定步进
    frame = modify_frame(frame, 0xff, eCMD_0x73, nsteps);
    if (!frame)
    {
        printf("modify frame failed.\n");
        return -1;
    } 

    ret = send_frame(frame);
    if (ret < 0)
    {
        printf("send frame failed.\n");
        return -1;
    }

    ret = rev_frame(frame);
    if (ret < 0)
    {
        printf("rcv frame failed.\n");
        return -1;
    }

    // 等待转动结束
    
    

    free_frame(frame);

    return nsteps;
}

int motor_stop(motor_id_t motor)
{
    return 0;
}

int get_motor_current_pos(motor_id_t motor, int *npos)
{
    return 0;
}

int set_motor_current_pos(motor_id_t motor, int npos)
{
    return 0;
}

int get_motor_status_1(motor_id_t motor, int *status)
{
    return 0;
}

int get_motor_status_2(motor_id_t motor, int *status)
{
    return 0;
}

void motor_exit(void)
{
    return ;
}
