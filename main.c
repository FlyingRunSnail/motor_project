#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "motor.h"

static void usage(void)
{
    printf("usage:\n \
            start 0 0 360  #start motor rotate 360 as C.W\n \
            stop 0          #stop motor rotate\n \
            get 0           #get motor current pos\n \
            set 0           #set motor current pos\n \
            status1 0       #get motor status 1\n \
            status2 0       #get motor status 2\n \
            \n");
}

int main(int argc, char **argv)
{
    int ret;
    motor_id_t motor;
    rotate_direct_t dir;
    int status = 0;
    float angles = 360.0f;
    float ret_angles = 0.0f;
    int npos = 0;
    
    if (argc < 3)
    {
        usage();
        return -1;
    }
    
    motor = atoi(argv[2]);
     
    motor_init();

    if (strcmp(argv[1], "start") == 0)
    {
        dir = atoi(argv[3]);
        angles = atof(argv[4]);
        ret = motor_start(motor, dir, angles, &ret_angles);
        printf("motor %d rotate %d steps %.2f\n", motor, dir, ret_angles);
    }
    else if (strcmp(argv[1], "stop") == 0)
    {
        ret = motor_stop(motor);
        printf("motor %d stop\n", motor);
    }
    else if (strcmp(argv[1], "get") == 0) 
    {
        ret = get_motor_current_pos(motor, &npos);
        printf("motor %d current pos %d and ret %d\n", motor, npos, ret);
    }
    else if (strcmp(argv[1], "set") == 0)
    {
        ret = set_motor_current_pos(motor, npos);
        printf("motor %d set current pos %d and ret %d\n", motor, npos, ret);
    }
    else if (strcmp(argv[1], "status1") == 0)
    {
        ret = get_motor_status_1(motor, &status);
        printf("status1: 0x%08x\n", status);
    }
    else if (strcmp(argv[1], "status2") == 0)
    {
        ret = get_motor_status_2(motor, &status);
        printf("status2: 0x%08x\n", status);
    }
    else
    {
        usage();
    }

    motor_exit();
    return 0;
}

