// PID regulator calculation


#include "V_Include/V_Pid.h"
#include "V_Include/V_IQmath.h"
#include "stdlib.h"

#pragma CODE_SECTION(Pid_Calc, "ramfuncs");

void Pid_Calc(TPid *p) {
    p->pid_err = p->pid_ref - p->pid_fdb;
    p->pid_errDZ = p->pid_err; //dead zone is currently not used

    p->pid_out_p = _IQmpy(p->pid_Kp, p->pid_errDZ);

    p->pid_out_i = p->pid_out_i + _IQmpy(p->pid_Ki, p->pid_errDZ);

    if(p->pid_out_i > p->pid_out_max){
        p->pid_out_i = p->pid_out_max;
    } else if (p->pid_out_i < p->pid_out_min){
        p->pid_out_i = p->pid_out_min;
    }

    p->pid_out = p->pid_out_p + p->pid_out_i;

    if(p->pid_out > p->pid_out_max){
        p->pid_out = p->pid_out_max;
    }else if (p->pid_out < p->pid_out_min){
        p->pid_out = p->pid_out_min;
    }
}

void Pid_Reset(TPid *p) {
    p->pid_err = 0;
    p->pid_ref = 0;
    p->pid_fdb = 0;
    p->pid_out_i = 0;
    p->pid_out = 0;
}
