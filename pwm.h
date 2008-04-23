#ifndef __PWM_H__
#define __PWM_H__

typedef struct pwm_manager {
	unsigned int ResOffCt, loop_length;
	unsigned short ADdelay, PWMVal, phase, Cmask, rs, rs2; 
	unsigned short ResOn, ResOnLocal, SweepDir, ResOffCtLocal;
} pwm_manager;

#endif
