#ifndef __MOTOR_DRIVER__
#define __MOTOR_DRIVER__

typedef enum{eMotor_h_dir = 0, 
             eMotor_v_dir, 
             eMotor_max}motor_id_t;

typedef enum{eDirClockWise = 0, 
             eDirAntiClockWise, 
             eDirMax}rotate_direct_t;

int motor_start(motor_id_t motor, rotate_direct_t dir, int nsteps);
int motor_stop(motor_id_t motor);
int get_motor_current_pos(motor_id_t motor, int *npos);
int set_motor_current_pos(motor_id_t motor, int npos);
int get_motor_status_1(motor_id_t motor, int *status);
int get_motor_status_2(motor_id_t motor, int *status);

#endif
