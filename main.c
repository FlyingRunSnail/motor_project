#include <stdio.h>
#include <string.h>
#include "motor.h"

static void usage(void)
{
    printf("usage:\
            start 0 0 3200  #start motor rotate 3200 as C.W\
            stop 0          #stop motor rotate\
            get 0           #get motor current pos\
            set 0           #set motor current pos\
            \r\n");
}

int main(int argc, char **argv)
{
    int ret;
    motor_id_t motor;
    rotate_direct_t dir;
    int nsteps = 3200;
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
        nsteps = atoi(argv[4]);
        ret = motor_start(motor, dir, nsteps);
        printf("motor %d rotate %d steps %d\n", motor, dir, ret);
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
    else
    {
        usage();
    }

    motor_exit();
    return 0;
}

