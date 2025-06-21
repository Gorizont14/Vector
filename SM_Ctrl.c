//
//Main control system state machine calculation
//

#include "V_Include/SM_Ctrl.h"
#include "V_Include/main.h"

#pragma CODE_SECTION(SM_Ctrl_Fast_Calc, "ramfuncs");
#pragma CODE_SECTION(SM_Ctrl_Slow_Calc, "ramfuncs");

unsigned int cnt_flt = 0; //initial driver fault wait counter

//Test variables
float   ctrltest1, ctrltest2, ctrltest3 = 0;
long    test_pid_id_out_max, test_pid_iq_out_max, test_pid_speed_out_max = 0;
long    test_pid_id_out_min, test_pid_iq_out_min, test_pid_speed_out_min = 0;
float   test_pid_id_out_max_hmi, test_pid_iq_out_max_hmi, test_pid_speed_out_max_hmi = 0;
float   test_pid_id_out_min_hmi, test_pid_iq_out_min_hmi, test_pid_speed_out_min_hmi = 0;
float   test_pid_speed_out_hmi, test_pid_id_out_hmi, test_pid_iq_out_hmi = 0;


// Control System init
void SM_Ctrl_Init(TSM_Ctrl *p)
{
    if (p->E == 0){
        pwm.off(&pwm);
        p->state = CTRL_INIT;
        p->E = 1;

        //pid regulators parameters init
        pid_speed.pid_Kp = _IQ(0.1);
        pid_speed.pid_Ki = _IQ(0.00005);
        pid_speed.pid_out_max = _IQ(drv_param.Imax); //limitation of maximum current for safety
        pid_speed.pid_out_min = - pid_speed.pid_out_max;

        pid_id.pid_Kp = _IQ(0.25);
        pid_id.pid_Ki = _IQ(0.00000005);
        pid_id.pid_out_max = _IQ(drv_param.Umax);
        pid_id.pid_out_min = - pid_id.pid_out_max;

        pid_iq.pid_Kp = _IQ(0.25);
        pid_iq.pid_Ki = _IQ(0.00000005);
        pid_iq.pid_out_max = _IQ(drv_param.Umax);
        pid_iq.pid_out_min = - pid_iq.pid_out_max;
    }
    //===================================================
    //system initialized
    p->state_prev = CTRL_INIT;
    p->state = CTRL_STOP;
}

void SM_Ctrl_Fast_Calc(TSM_Ctrl *p) // Fast calc on PWM frequency
{
    //if system is not yet initialized - skip control system
    if (sm_sys.state == SYS_INIT) {
        return;
    }

    if (p->state_prev != p->state)
        p->E = 1;
    else
        p->E = 0;
    p->state_prev = p->state;

    //===================================
    //Check of CW
    if (cw.bit.start == 1 && cw.bit.stop != 1) p->state = CTRL_RUNNING;
    if (cw.bit.stop == 1 || cw.bit.start == 0) p->state = CTRL_STOP;
    //===================================

    //Main state machine
    switch (p->state) {

    case CTRL_STOP: { //State of stopped drive - no pwm, waiting for state change
        if (p->E == 1) { //if first entry

        }

        pwm.off(&pwm);

        //Zeroing of everything dangerous
        pwm.Ualpha_ref = 0;
        pwm.Ubeta_ref = 0;
        pwm.update(&pwm);
        pid_speed.reset(&pid_speed);
        pid_id.reset(&pid_id);
        pid_iq.reset(&pid_iq);
        curpar.Isd = 0;
        curpar.Isq = 0;
        curpar.speed = 0;

        cnt_flt = 0;

        test_pid_speed_out_max  = 0;
        test_pid_id_out_max     = 0;
        test_pid_iq_out_max     = 0;
        test_pid_speed_out_min  = 0;
        test_pid_id_out_min     = 0;
        test_pid_iq_out_min     = 0;

        break;
        }

    case CTRL_STARTING: { //check for driver fault
        if (p->E == 1) { //if first entry

        }
        if (prot.drv_flt == 0) {
            break;                      //while driver is in fault, it does not provide current active fdb signal,
        }                               //which immediately saturates the current controller and ruins everything)

        p->state = CTRL_RUNNING;
        break;
    }

    case CTRL_RUNNING: {// drive is modulating
        if (p->E == 1) { //if first entry
            pwm.on(&pwm);
            rmp.out = 0;
        }

        //check for hw fault signal from driver
        if (cnt_flt < 5000) {
            pid_speed.reset(&pid_speed);
            pid_id.reset(&pid_id);
            pid_iq.reset(&pid_iq);
            cnt_flt++;
            break;
        }

        //gain currents in dq frame
        //calculation is done in IQ format in absolute values of A, V, rpm etc.

        //phase transform Currents
        clarke.as = adc.Iameas;
        clarke.bs = adc.Ibmeas;
        clarke.calc(&clarke);

        //Park transform Currents
        park.ds = clarke.ds;
        park.qs = clarke.qs;
        park.ang = curpar.Theta_electr;
        park.calc(&park);
        curpar.Isd = park.de;
        curpar.Isq = park.qe;

        //speed ramp generator
        rmp.input = refs.speed_ref;
        rmp.calc(&rmp);

        //speed regulator
        pid_speed.pid_ref = rmp.out;
        pid_speed.pid_fdb = curpar.speed;
        pid_speed.calc(&pid_speed);

        //Current regulators
        pid_id.pid_ref = 0;     //no magnetization for PMSM
        pid_id.pid_fdb = curpar.Isd;
        pid_id.calc(&pid_id);

        pid_iq.pid_ref = pid_speed.pid_out;
        pid_iq.pid_fdb = curpar.Isq;
        pid_iq.calc(&pid_iq);

        //Inverse park transform for I regs outputs -> Uref alpha/beta
        ipark.de = pid_id.pid_out;
        ipark.qe = pid_iq.pid_out;
        ipark.ang = curpar.Theta_electr;
        ipark.calc(&ipark);

        //Forming refs to Uabc through alha-beta refs
        pwm.Ualpha_ref = ipark.ds;
        pwm.Ubeta_ref  = ipark.qs;
        //pwm.Ualpha_ref = _IQ(ctrltest1);
        //pwm.Ubeta_ref = _IQ(ctrltest2);
        pwm.update(&pwm);
    }
    }
}

void SM_Ctrl_Khz_Calc(TSM_Ctrl *p) // Calc at fixed frequency of 1 kHz
{

}

void SM_Ctrl_Slow_Calc(TSM_Ctrl *p) // Off-real-time background calc
{
    //Refs recalc from SI units to pu IQ
    if(refs.speed_ref_hmi > drv_param.Speed_max_hmi){
        refs.speed_ref_hmi = drv_param.Speed_max_hmi;
    }else if (refs.speed_ref_hmi < 0){
        refs.speed_ref_hmi = 0; //only positive direction as for now
    }
    refs.speed_ref = _IQ(refs.speed_ref_hmi / drv_param.Speedmot_nom_hmi);

    test_pid_speed_out_hmi = _IQ24toF(pid_speed.pid_out);
    test_pid_id_out_hmi = _IQ24toF(pid_id.pid_out);
    test_pid_iq_out_hmi = _IQ24toF(pid_iq.pid_out);
}
