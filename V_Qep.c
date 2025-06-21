//
//Encoder Calc
//

#include "V_Include/V_Qep.h"
#include "V_Include/main.h"

#pragma CODE_SECTION(Qep_Khz_Calc, "ramfuncs");

//Line-driven (with separate supply and thus active out), single-ended (just 1 positive signal, no neg duplicate) TTL (5 V RS422) encoder
#define QEP_RESOLVE 4000 //QAB (QCLK) per revolution; UEVENT = 1 QAB
#define MIN_SPEED   0.165 //minimum sensible speed in rpm
#define MAX_SPEED   3000.0//max alowable speed in rpm
#define DIR_CW      1
#define DIR_CCW     -1

//___ Positive direction - by the left hand, CCW facing DE end ___
signed long Delta_pos = 0;

//+++ Test vars +++
float   test_max_speed = 0;


void Qep_Init(TQep *p)
{
        EALLOW;

        //+++ GPIO +++
        //EQEP2A - GPIO54

        //+++ Quadrature +++
        EQep2Regs.QDECCTL.bit.QAP   = 0;
        EQep2Regs.QDECCTL.bit.QBP   = 0;
        EQep2Regs.QDECCTL.bit.QIP   = 0;
        EQep2Regs.QDECCTL.bit.QSP   = 0;
        EQep2Regs.QDECCTL.bit.IGATE = 0;
        EQep2Regs.QDECCTL.bit.SWAP  = 0;
        EQep2Regs.QDECCTL.bit.XCR   = 0;
        EQep2Regs.QDECCTL.bit.QSRC  = 0; //Quadrature count mode (QCLK = iCLK, QDIR = iDIR)
        EQep2Regs.QDECCTL.bit.SOEN  = 0; //Position cmp -> I is OFF
        EQep2Regs.QDECCTL.bit.SPSEL = 0;

        //+++ Position +++
        EQep2Regs.QEPCTL.bit.FREE_SOFT  = 3; // QEP operation continues on suspend
        EQep2Regs.QEPCTL.bit.PCRM       = 0; // position reset on I event
        EQep2Regs.QEPCTL.bit.IEI        = 2; // init position on rising edge of I

        EQep2Regs.QPOSINIT              = 0; // Value of initial position
        EQep2Regs.QPOSMAX               = QEP_RESOLVE - 1; // Max cnt encoder value: 4000-1
        EQep2Regs.QEPCTL.bit.SWI        = 1; // Init pos cnt
        EQep2Regs.QEPCTL.bit.SWI        = 0; // Clear bit
        EQep2Regs.QEPCTL.bit.SEL        = 0; // Pos latch on Strobe - not used
        EQep2Regs.QEPCTL.bit.IEL        = 2; // Latch of pos & dir on I

        EQep2Regs.QEPCTL.bit.QPEN       = 1; // Enable pos cnt
        EQep2Regs.QEPCTL.bit.UTE        = 0; // Enable unit timer for UTO - not used
        EQep2Regs.QEPCTL.bit.WDE        = 0; // Enable watchdog - not used
        EQep2Regs.QPOSCTL.all           = 0; // Pos cmp - OFF, not used

        //+++ QEP Edge capture unit +++

        //+++ Max speed +++
        //Max speed - 3000 rpm = 50 rev/s = 50 Hz; resolution = 4000 qlck/rev
        //Max qlck/s = 50 x 4000 = 200 000 qlck/s;
        //QCLK = 200 000 1/s = 200 kHz
        //At 10 kHz PWM it will be 200 kHz / 10 kHz = 20 qclk / PWM period - needs a filter with at least x5 freq div

        //+++ Min speed +++
        // 65535 / 703125 = 0.0932 s - max time between qclk ~= 11 qclk / s
        // 11 qclk / 4000 = 0.00275 rev/s = 0.165 rev/min
        //Everything below will be considered as zero speed

        EQep2Regs.QCAPCTL.bit.CCPS  = 7; //CAPCLK = SYSCLKOUT/128 = 90 MHz / 128 = 703 125 Hz
        p->QCTMR_CLK = SYSCLKOUT / 128;  //703 125 Hz
        EQep2Regs.QCAPCTL.bit.UPPS  = 0; //UPEVNT = QCLK - event on each QAB tick
        p->UPEVNT_Q = 1;
        EQep2Regs.QCAPCTL.bit.CEN   = 1; //Enable QCAP
        //Low speed control - by calculating time (ticks) between UPEVNT
        EQep2Regs.QEPCTL.bit.QCLM   = 0; //1 - Out latched to QPOSLAT, QCTMRLAT and QCPRDLAT on UTM timer


        //+++ Interrupt +++
        //QEPSTS:UPEVNT -> dT = QCPRD
        //QCTMR up to 65,535 - overflow -> QEPSTS.COEF
        //QFLG.WTO - watch dog timeout, period = SYSCLKOUT/64, resets by QA/QB

        //Interrupt to calc speed at fixed freq equal to PWM freq
        EQep2Regs.QUPRD = SYSCLKOUT / pwm.Pwm_freq;
        EQep2Regs.QEINT.bit.UTO = 0;

        EDIS;
}

void Qep_Fast_Calc(TQep *p)
{

}

void Qep_Khz_Calc(TQep *p)
{
    // Main QEP Calc on PWM freq
    //Incremental position output and incremental time output is available in the QPOSLAT and QCPRDLAT registers.

    //Low Speed Calc only - for Test!

    //Direction check
    switch (EQep2Regs.QEPSTS.bit.QDF) {
    case 0: //CW viewing from D-end, position increments
        p->Direction = DIR_CW;
    break;
    case 1: //CCW viewing from D-end, position decrements
        p->Direction = DIR_CCW;
    break;
    }

    //Check for direction change: if yes - speed = 0 and no actual speed calc
    if(EQep2Regs.QEPSTS.bit.CDEF == 0){

        //--- Low Speed Calc ---
        if(EQep2Regs.QEPSTS.bit.COEF == 0){// - timer not overflowed and no dir change
            if (EQep2Regs.QEPSTS.bit.UPEVNT != 0){
                p->Speed_Low = 60.0 / (((EQep2Regs.QCPRD * 1.0) / p->QCTMR_CLK) * QEP_RESOLVE); //rpm
                EQep2Regs.QEPSTS.bit.UPEVNT = 1;//clear flag
            }
        }
        else {
            p->Speed_Low = 0;
            EQep2Regs.QEPSTS.bit.COEF = 1;//clear flag timer overflow
            EQep2Regs.QEPSTS.bit.CDEF = 1;//clear flag dir change
        }

    } else { //dir has changed - speed is considered to be zero
        p->Speed_Low = 0;
        EQep2Regs.QEPSTS.bit.CDEF = 1;//clear flag dir change
    }

    //angular position recalc to pu
    curpar.Theta_mech = _IQdiv(EQep2Regs.QPOSCNT, QEP_RESOLVE);
    curpar.Theta_mech &= 0x00FFFFFF; //cut over 1 in IQ8.24 - 1 rotate = 360 deg = 1 pu = 1.0 in IQ8.24
    curpar.Theta_electr = curpar.Theta_mech << 2; //multiply by drv_param.pmot - motor pole pairs
    curpar.Theta_electr &= 0x00FFFFFF;

    //speed recalc to pu
    curpar.speed_hmi = p->Speed_Low * p->Direction; //current speed in real rpm
    if(curpar.speed_hmi > test_max_speed) test_max_speed = curpar.speed_hmi;
    curpar.speed = _IQ(curpar.speed_hmi / drv_param.Speedmot_nom_hmi); //make speed signed in pu IQ: - CW, + CCW
}

void Qep_Slow_Calc(TQep *p)
{
    //Recalc of nominal coefficient
}

__interrupt void EQEP2_INT_Handler(void)
{
    //not used - calc is done in Fast Calc on PWM freq
    //check flags
    int flags = EQep2Regs.QFLG.all;
    if ((flags & 0x07FE) != 0){
        //check for other flags excl INT and UTO
    }
    //do calc
    EQep2Regs.QPOSLAT; // - latched value of QPOS to work with in this INT handler

    //reset flags
    EQep2Regs.QCLR.bit.UTO = 1;
    EQep2Regs.QCLR.bit.INT = 1;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP5;
}

void Full_Speed_Calc(TQep *p)
{
    // Main QEP Calc on PWM freq
    //Incremental position output and incremental time output is available in the QPOSLAT and QCPRDLAT registers.

    //First entry
    if (p->E == 0){
        p->Position_prev = EQep2Regs.QPOSCNT;
        p->E = 1;
    }

    //Direction check
    switch (EQep2Regs.QEPSTS.bit.QDF) {
    case 0: //CW viewing from D-end, position increments
        p->Direction = DIR_CW;
    break;
    case 1: //CCW viewing from D-end, position decrements
        p->Direction = DIR_CCW;
    break;
    }
    //QDLF for dir on I

    //Frequency divisor for High speed calc, relative to Fast Calc = PWM Freq
    if (p->Filter_div != 0) p->Filter_cnt++;

    //Check for direction change: if yes - speed = 0 and no actual speed calc
    if(EQep2Regs.QEPSTS.bit.CDEF == 0){

        //--- Low Speed Calc ---
        if(EQep2Regs.QEPSTS.bit.COEF == 0 && EQep2Regs.QEPSTS.bit.CDEF == 0){// - timer not overflowed and no dir change
            if (EQep2Regs.QEPSTS.bit.UPEVNT != 0){
                p->Period = EQep2Regs.QCPRD;
                p->Speed_Low = 60.0 / (((p->Period * 1.0) / p->QCTMR_CLK) * QEP_RESOLVE);
                EQep2Regs.QEPSTS.bit.UPEVNT = 1;//clear flag
            }
        }
        else {
            p->Speed_Low = 0.0;
            EQep2Regs.QEPSTS.bit.COEF = 1;//clear flag timer overflow
            EQep2Regs.QEPSTS.bit.CDEF = 1;//clear flag dir change
        }

        //--- High Speed Calc ---
        if (p->Filter_cnt >= p->Filter_div){
            p->Position = EQep2Regs.QPOSCNT;//reading QPOSCNT latches current QCPRD value into QCPRDLAT for Low Speed calc

            //Check for going over Index - in this case delta position will be negative, like 28 - 3997
            // ! When direction changes around zero speed, direction can change due to some small back movement (magnets)
            // before reading of new position, thus a separate check for match between direction and delta_pos is needed.
            // In fact this does not matter due to this can occur only around zero speed, where High speed calc is not used,
            // but for program safety it's better to implement an additional check

            Delta_pos = p->Position - p->Position_prev;

            if(Delta_pos < (500 - QEP_RESOLVE) && p->Direction == DIR_CCW){
                Delta_pos = Delta_pos + QEP_RESOLVE;
            } else if(Delta_pos > (QEP_RESOLVE - 500) && p->Direction == DIR_CW){
                Delta_pos = Delta_pos - QEP_RESOLVE;
            }
            //patch for limiting wrong value of Delta_pos due to backward movement around zero speed
            if(Delta_pos > (QEP_RESOLVE - 500) || Delta_pos < (500 - QEP_RESOLVE)) Delta_pos = 0;

            if(p->Filter_div > 0){
                p->Speed_High = (Delta_pos * p->Direction * pwm.Pwm_freq * 60.0 / p->Filter_div) / QEP_RESOLVE;
            }else {
                p->Speed_High = (Delta_pos * p->Direction * pwm.Pwm_freq * 60.0) / QEP_RESOLVE;
            }
            p->Position_prev = p->Position;
            p->Filter_cnt = 0;
        }
    } else {
        p->Speed_Low = 0.0;
        p->Speed_High = 0.0;
        EQep2Regs.QEPSTS.bit.CDEF = 1;//clear flag dir change
    }
    //--- Speed Calc ---
    //Switchover to High speed with Filter div = 125 and 40 fb read per rev:
    //Low speed - 93.75 TMR clk between QLCK, error == 1.07% - Fine
    //High speed - 93.75 pos change per each fb read cycle, error == 1.07% - Fine
    //With fixed number of read cycles per rpm and thus adjusted filter div high speed calc error remains constant

    if (p->Speed_Low <= p->Switch_Speed){
        p->Speed = p->Speed_Low;
        //p->Speed_High = p->Speed_Low; //to keep bumpless switchover to High Speed for a time before first High Speed calc
    } else {
        p->Speed = p->Speed_High;
    }
    p->Speed = p->Speed * p->Direction; //make speed signed: - CW, + CCW

    if (p->Speed > p->Speed_Max_Measured && p->Speed) p->Speed_Max_Measured = p->Speed;
    if (p->Speed_Low > Speed_Max_Low) {
        Speed_Max_Low = p->Speed_Low;
    }

    p->SpeedIQ = _IQ(p->Speed);
}
