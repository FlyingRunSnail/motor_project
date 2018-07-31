#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include "motor.h"
#include "rs485.h"
#include "protocol.h"

// 步进角
#define ANGLES_PER_STEP    (1.8f)
// 细分数
#define MOTOR_SUBDIV_COUNT (16)

static int motor_send_and_check(unsigned char addr, unsigned char command, int data, unsigned char check, int *pdata)
{
    int ret;
    comm_frame_t *frame;

    // 构造命令帧
    frame = alloc_frame(addr, command, data);
    if (!frame)
    {
        printf("call alloc_frame failed, addr: 0x%x command: 0x%x data: 0x%08x.\n", addr, command, data);
        return -1;
    }

    ret = send_frame(frame);
    if (ret < 0)
    {
        printf("send frame failed.\n");
        free_frame(frame);
        return -1;
    }

    ret = rcv_frame(frame);
    if (ret < 0)
    {
        printf("rcv frame failed.\n");
        free_frame(frame);
        return -1;
    }

    if (check)
    {
	if (frame->data != data)
	{
	    printf("set motor data failed, requested 0x%x and return value 0x%x.\n", data, frame->data);
	    dump_frame(frame);
	    free_frame(frame);
	    return -1;
	}
    }

    if (pdata != NULL)
    {
        *pdata = frame->data;
    }

    free_frame(frame);

    return 0;
}

static int motor_is_busy(motor_id_t motor, int *status)
{
    int ret;

    if (!status)
    {
        printf("input param status is illegal.\n");
        return -1;
    }

    *status = 0;
    
    // 构造命令帧
    ret = motor_send_and_check(0xff, eCMD_0x6a, 0x0, 0, status);
    if (ret < 0)
    {
        printf("send frame failed.\n");
        return -1;
    }

    // 控制器是否BUSY
    if (*status & 0x1)
    {
        *status = 0x1;
    }
    else
    {
        *status = 0x0;
    }

    return 0;
}

// 水平方向motor初始化
static int motor_h_init(void)
{
    int status = 0;
    int ret;
    int stall_config = 0x3;
    int stall_trigger = 0xff;
    int stall_length = 0x40;
    int stall_value = 0x0;

    // 构造读取状态命令帧
    ret = motor_is_busy(eMotor_h_dir, &status);
    if (ret < 0)
    {
	printf("call motor_is_busy failed.\n");
        return -1;
    }

    if (status == 0x1)
    {
	printf("motor is busy.\n");
        return -1;
    }

    // 设定细分数
    ret = motor_send_and_check(0xff, eCMD_0x6d, MOTOR_SUBDIV_COUNT, 1, NULL);
    if (ret != 0)
    {
        printf("set motor sub div failed.\n");
	return -1;
    }

#if 0
    // 设定当前位置
    ret = motor_send_and_check(0xff, eCMD_0x69, 0x0, NULL);
    if (ret != 0x0)
    {
	printf("set motor current pos failed.\n");
        return -1;
    }
#endif

    // 设定转动方向 
    ret = motor_send_and_check(0xff, eCMD_0x64, 0x0, 1, NULL);
    if (ret != 0x0)
    {
	printf("set motor rotate direction failed.\n");
        return -1;
    }

    // 设定最高转速, 4000
    ret = motor_send_and_check(0xff, eCMD_0x76, 0x0fa0, 1, NULL);
    if (ret != 0x0)
    {
	printf("set motor max speed failed.\n");
        return -1;
    }

    // 设定速度补偿因子, 1500
    ret = motor_send_and_check(0xff, eCMD_0x50, 0x05dc, 1, NULL);
    if (ret != 0x0)
    {
	printf("set motor speed compensation factor failed.\n");
        return -1;
    }

    // 设定启动速度, 600
    ret = motor_send_and_check(0xff, eCMD_0x4c, 0x0258, 1, NULL);
    if (ret != 0x0)
    {
	printf("set motor start speed failed.\n");
        return -1;
    }

    // 设定停止速度
    ret = motor_send_and_check(0xff, eCMD_0x53, 0x0258, 1, NULL);
    if (ret != 0x0)
    {
	printf("set motor stop speed failed.\n");
        return -1;
    }

    // 设定加速度系数
    ret = motor_send_and_check(0xff, eCMD_0x75, 0x03, 1, NULL);
    if (ret != 0x0)
    {
	printf("set motor stop speed failed.\n");
        return -1;
    }

    // 设定减速度系数
    ret = motor_send_and_check(0xff, eCMD_0x6f, 0x03, 1, NULL);
    if (ret != 0x0)
    {
	printf("set motor stop speed failed.\n");
        return -1;
    }

    // 设定最大相电流
    ret = motor_send_and_check(0xff, eCMD_0x65, 0x05dc, 1, NULL);
    if (ret != 0x0)
    {
	printf("set motor stop speed failed.\n");
        return -1;
    }

    // 设定电流补偿因子
    ret = motor_send_and_check(0xff, eCMD_0x4d, 0x012c, 1, NULL);
    if (ret != 0x0)
    {
	printf("set motor stop speed failed.\n");
        return -1;
    }

    // 设定外部紧急停止使能
    ret = motor_send_and_check(0xff, eCMD_0x66, 0x0, 1, NULL);
    if (ret != 0x0)
    {
	printf("set motor stop speed failed.\n");
        return -1;
    }

    // 设定外部触发方式
    ret = motor_send_and_check(0xff, eCMD_0x4a, 0x0, 1, NULL);
    if (ret != 0x0)
    {
	printf("set motor stop speed failed.\n");
        return -1;
    }

    // 设定自动电流衰减使能
    ret = motor_send_and_check(0xff, eCMD_0x61, 0x01, 1, NULL);
    if (ret != 0x0)
    {
	printf("set motor stop speed failed.\n");
        return -1;
    }

    // 设定自动电流衰减系数
    ret = motor_send_and_check(0xff, eCMD_0x51, 0x01, 1, NULL);
    if (ret != 0x0)
    {
	printf("set motor stop speed failed.\n");
        return -1;
    }

    // 设定脱机使能
    ret = motor_send_and_check(0xff, eCMD_0x67, 0x00, 1, NULL);
    if (ret != 0x0)
    {
	printf("set motor stop speed failed.\n");
        return -1;
    }

    // 设定堵转配置
    //TODO:

    // 写堵转配置寄存器
    ret = motor_send_and_check(0xff, eCMD_0x59, stall_config, 1, NULL);
    if (ret != 0x0)
    {
        printf("set motor stall configuration register failed.\n");
        return -1;
    }

    // 写堵转触发值
    ret = motor_send_and_check(0xff, eCMD_0x5a, stall_trigger, 1, NULL);
    if (ret != 0x0)
    {
        printf("set motor stall configuration register failed.\n");
        return -1;
    }

    // 写堵转触发长度
    ret = motor_send_and_check(0xff, eCMD_0x54, stall_length, 1, NULL);
    if (ret != 0x0)
    {
        printf("set motor stall configuration register failed.\n");
        return -1;
    }

    // 读堵转配置寄存器
    ret = motor_send_and_check(0xff, eCMD_0x59, 0xa, 0, &stall_value);
    if (ret != 0x0)
    {
        printf("get motor stall configuration register failed.\n");
        return -1;
    }

    if (stall_value != stall_config)
    {
        printf("read back motor stall configuration register 0x%x and write value 0x%x.\n", stall_value, stall_config);
        return -1;
    }

    // 读堵转触发值
    ret = motor_send_and_check(0xff, eCMD_0x5a, 0x200, 0, &stall_value);
    if (ret != 0x0)
    {
        printf("get motor stall trigger value failed.\n");
        return -1;
    }

    if (stall_value != stall_trigger)
    {
        printf("read back motor stall trigger value 0x%x and write value 0x%x.\n", stall_value, stall_trigger);
        return -1;
    }

    // 读堵转触发长度
    ret = motor_send_and_check(0xff, eCMD_0x54, 0x41, 0, &stall_value);
    if (ret != 0)
    {
        printf("get motor stall length value failed.\n");
        return -1;
    }

    if (stall_value != stall_length)
    {
	printf("read back motor stall length value 0x%x and write value 0x%x.\n", stall_value, stall_length);
        return -1;
    }

    return 0;
}

// 垂直方向motor初始化
static int motor_v_init(void)
{
#if 0
    int status = 0;
    int ret;
    int stall_config = 0x3;
    int stall_trigger = 0xff;
    int stall_length = 0x40;
    int stall_value = 0x0;

    // 构造读取状态命令帧
    ret = motor_is_busy(eMotor_v_dir, &status);
    if (ret < 0)
    {
	printf("call motor_is_busy failed.\n");
        return -1;
    }

    if (status == 0x1)
    {
	printf("motor is busy.\n");
        return -1;
    }

    // 设定细分数
    ret = motor_send_and_check(0xff, eCMD_0x6d, MOTOR_SUBDIV_COUNT, 1, NULL);
    if (ret != 0)
    {
        printf("set motor sub div failed.\n");
	return -1;
    }

#if 0
    // 设定当前位置
    ret = motor_send_and_check(0xff, eCMD_0x69, 0x0, 1, NULL);
    if (ret != 0x0)
    {
	printf("set motor current pos failed.\n");
        return -1;
    }
#endif

    // 设定转动方向 
    ret = motor_send_and_check(0xff, eCMD_0x64, 0x0, 1, NULL);
    if (ret != 0x0)
    {
	printf("set motor rotate direction failed.\n");
        return -1;
    }

    // 设定最高转速, 4000
    ret = motor_send_and_check(0xff, eCMD_0x76, 0x0fa0, 1, NULL);
    if (ret != 0x0)
    {
	printf("set motor max speed failed.\n");
        return -1;
    }

    // 设定速度补偿因子, 1500
    ret = motor_send_and_check(0xff, eCMD_0x50, 0x05dc, 1, NULL);
    if (ret != 0x0)
    {
	printf("set motor speed compensation factor failed.\n");
        return -1;
    }

    // 设定启动速度, 600
    ret = motor_send_and_check(0xff, eCMD_0x4c, 0x0258, 1, NULL);
    if (ret != 0x0)
    {
	printf("set motor start speed failed.\n");
        return -1;
    }

    // 设定停止速度
    ret = motor_send_and_check(0xff, eCMD_0x53, 0x0258, 1, NULL);
    if (ret != 0x0)
    {
	printf("set motor stop speed failed.\n");
        return -1;
    }

    // 设定加速度系数
    ret = motor_send_and_check(0xff, eCMD_0x75, 0x03, 1, NULL);
    if (ret != 0x0)
    {
	printf("set motor stop speed failed.\n");
        return -1;
    }

    // 设定减速度系数
    ret = motor_send_and_check(0xff, eCMD_0x6f, 0x03, 1, NULL);
    if (ret != 0x0)
    {
	printf("set motor stop speed failed.\n");
        return -1;
    }

    // 设定最大相电流
    ret = motor_send_and_check(0xff, eCMD_0x65, 0x05dc, 1, NULL);
    if (ret != 0x0)
    {
	printf("set motor stop speed failed.\n");
        return -1;
    }

    // 设定电流补偿因子
    ret = motor_send_and_check(0xff, eCMD_0x4d, 0x012c, 1, NULL);
    if (ret != 0x0)
    {
	printf("set motor stop speed failed.\n");
        return -1;
    }

    // 设定外部紧急停止使能
    ret = motor_send_and_check(0xff, eCMD_0x66, 0x0, 1, NULL);
    if (ret != 0x0)
    {
	printf("set motor stop speed failed.\n");
        return -1;
    }

    // 设定外部触发方式
    ret = motor_send_and_check(0xff, eCMD_0x4a, 0x0, 1, NULL);
    if (ret != 0x0)
    {
	printf("set motor stop speed failed.\n");
        return -1;
    }

    // 设定自动电流衰减使能
    ret = motor_send_and_check(0xff, eCMD_0x61, 0x01, 1, NULL);
    if (ret != 0x0)
    {
	printf("set motor stop speed failed.\n");
        return -1;
    }

    // 设定自动电流衰减系数
    ret = motor_send_and_check(0xff, eCMD_0x51, 0x01, 1, NULL);
    if (ret != 0x0)
    {
	printf("set motor stop speed failed.\n");
        return -1;
    }

    // 设定脱机使能
    ret = motor_send_and_check(0xff, eCMD_0x67, 0x00, 1, NULL);
    if (ret != 0x0)
    {
	printf("set motor stop speed failed.\n");
        return -1;
    }

    // 设定堵转配置
    //TODO:

    // 写堵转配置寄存器
    ret = motor_send_and_check(0xff, eCMD_0x59, stall_config, 1, NULL);
    if (ret != 0x0)
    {
        printf("set motor stall configuration register failed.\n");
        return -1;
    }

    // 写堵转触发值
    ret = motor_send_and_check(0xff, eCMD_0x5a, stall_trigger, 1, NULL);
    if (ret != 0x0)
    {
        printf("set motor stall configuration register failed.\n");
        return -1;
    }

    // 写堵转触发长度
    ret = motor_send_and_check(0xff, eCMD_0x54, stall_length, 1, NULL);
    if (ret != 0x0)
    {
        printf("set motor stall configuration register failed.\n");
        return -1;
    }

    // 读堵转配置寄存器
    ret = motor_send_and_check(0xff, eCMD_0x59, 0xa, 0, &stall_value);
    if (ret != 0x0)
    {
        printf("get motor stall configuration register failed.\n");
        return -1;
    }

    if (stall_value != stall_config)
    {
        printf("read back motor stall configuration register 0x%x and write value 0x%x.\n", stall_value, stall_config);
        return -1;
    }

    // 读堵转触发值
    ret = motor_send_and_check(0xff, eCMD_0x5a, 0x200, 0, &stall_value);
    if (ret != 0x0)
    {
        printf("get motor stall trigger value failed.\n");
        return -1;
    }

    if (stall_value != stall_trigger)
    {
        printf("read back motor stall trigger value 0x%x and write value 0x%x.\n", stall_value, stall_trigger);
        return -1;
    }

    // 读堵转触发长度
    ret = motor_send_and_check(0xff, eCMD_0x54, 0x41, 0, &stall_value);
    if (ret != 0)
    {
        printf("get motor stall length value failed.\n");
        return -1;
    }

    if (stall_value != stall_length)
    {
	printf("read back motor stall length value 0x%x and write value 0x%x.\n", stall_value, stall_length);
        return -1;
    }
#endif

    return 0;
}

int motor_init(void)
{
    int ret;

    printf("motor init start...\n");
    rs485_init("/dev/ttyUSB0");

    // 马达控制器参数设置
    // TODO:
    ret = motor_h_init();
    if (ret < 0)
    {
	printf("motor_h_init failed.\n");
        return -1;
    }

    ret = motor_v_init();
    if (ret < 0)
    {
	printf("motor_v_init failed.\n");
        return -1;
    }

    printf("motor init end...\n");
    return 0;
}


static int motor_start_internal(motor_id_t motor, rotate_direct_t dir, int nsteps)
{
    int ret;
    
    // 设定转动方向
    ret = motor_send_and_check(0xff, eCMD_0x64, dir, 1, NULL);
    if (ret != 0)
    {
        printf("set motor rotate dir failed.\n");
        return -1;
    }

    // 设置转动给定步长
    ret = motor_send_and_check(0xff, eCMD_0x73, nsteps, 1, NULL);
    if (ret != 0)
    {
        printf("start motor rotate %d steps failed.\n", nsteps);
        return -1;
    } 

    return nsteps;
}

int motor_start(motor_id_t motor, rotate_direct_t dir, float angles, float *ret_angles)
{
    int nsteps;
    int ret;
    int his_pos = 0;
    int new_pos = 0;
    int status = 0;
    
    if (!ret_angles)
    {
        printf("output param ret_angles is invalid.\n");
	return -1;
    }

    if (angles < 0.0f)
    {
	printf("input param angles is invalid.\n");
        return -1;
    }

    ret = motor_is_busy(motor, &status);
    if (ret < 0)
    {
	printf("call motor_is_busy failed.\n");
        return -1;
    }

    if (status == 0x1)
    {
	printf("motor is busy.\n");
        return -1;
    }
    
    nsteps = (int)roundf((angles * (float)(MOTOR_SUBDIV_COUNT)) / ANGLES_PER_STEP);
    printf("angles %.2f and steps %d\n", angles, nsteps);

    ret = get_motor_current_pos(motor, &his_pos);
    if (ret < 0)
    {
        return ret;
    }

    printf("motor start pos %d.\n", his_pos);

    ret = motor_start_internal(motor, dir, nsteps);
    if (ret < 0)
    {
        return ret;
    }
        
    while(1)
    {
        ret = get_motor_status_1(motor, &status);
        if (ret < 0)
        {
            continue;
        }

	if ((status & 0x1) == 0x1)
	{
	    usleep(500*1000);
	    continue;
	}

	break;
    }

    if ((status & 0x10) == 0x10)
    {
        ret = get_motor_stall_pos(motor, &new_pos);
	if (ret < 0)
	{
	    printf("get motor stall pos failed.\n");
	    return -1;
	}
    }
    else
    {
	ret = get_motor_current_pos(motor, &new_pos);
	if (ret < 0)
	{
	    printf("get motor current pos failed.\n");
	    return -1;
	}
    }

    *ret_angles = (float)(new_pos - his_pos) * ANGLES_PER_STEP / MOTOR_SUBDIV_COUNT;

    printf("motor finish rotate and current pos %d.\n", new_pos);
    printf("motor rotate %d steps and angles %.2f\n", new_pos - his_pos, *ret_angles);

    return 0;
}

int motor_stop(motor_id_t motor)
{
    int ret;

    // 根据读取命令构造命令帧
    ret = motor_send_and_check(0xff, eCMD_0x49, 0, 1, NULL);
    if (ret != 0)
    {
        printf("stop motor failed.\n");
        return -1;
    }

    printf("stop motor success.\n");

    return 0;
}

int get_motor_stall_pos(motor_id_t motor, int *npos)
{
    int ret;

    if (!npos)
    {
        printf("output param npos is invalid.\n");
        return -1;
    }

    *npos = 0;

    // 根据读取命令构造命令帧
    ret = motor_send_and_check(0xff, eCMD_0x58, 0, 0, npos);
    if (ret < 0)
    {
        printf("get motor stall pos failed.\n");
        return -1;
    }

    return 0;
}

int get_motor_current_pos(motor_id_t motor, int *npos)
{
    int ret;

    if (!npos)
    {
        printf("output param npos is invalid.\n");
        return -1;
    }

    *npos = 0;

    // 根据读取命令构造命令帧
    ret = motor_send_and_check(0xff, eCMD_0x63, 0, 0, npos);
    if (ret < 0)
    {
        printf("get motor current pos failed.\n");
        return -1;
    }

    return 0;
}

int set_motor_current_pos(motor_id_t motor, int npos)
{
    int ret;

    if (npos < 0)
    {
	printf("input param npos is invalid, must be at [0x0~0x7fffffff].\n");
        return -1;
    }

    // 根据读取命令构造命令帧
    ret = motor_send_and_check(0xff, eCMD_0x69, npos, 0, NULL);
    if (ret < 0)
    {
        printf("set motor pos to %d failed.\n", npos);
        return -1;
    }

    return 0;
}

int get_motor_status_1(motor_id_t motor, int *status)
{
    int ret;

    if (!status)
    {
        printf("output param status is invalid.\n");
        return -1;
    }

    *status = 0;

    // 构造命令帧
    ret = motor_send_and_check(0xff, eCMD_0x6a, 0x0, 0, status);
    if (ret < 0)
    {
        printf("get status1 failed.\n");
        return -1;
    }

    return 0;
}

int get_motor_status_2(motor_id_t motor, int *status)
{
    int ret;

    if (!status)
    {
        printf("output param status is invalid.\n");
        return -1;
    }

    *status = 0;

    // 根据读取命令构造命令帧
    ret = motor_send_and_check(0xff, eCMD_0x4f, 0x0, 0, status);
    if (ret < 0)
    {
        printf("get status2 failed.\n");
        return -1;
    }

    return 0;
}

void motor_exit(void)
{
    rs485_exit();
    return ;
}

