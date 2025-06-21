//###########################################################################
//
// FILE:	My main - Main file of the program, includes main() function
//
//###########################################################################

//
// Included Files
//
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <file.h>

#include "DSP28x_Project.h"     // DSP28x Headerfile

#include "V_Include/main.h"     // main header frame file
#include <string.h>             // memcopy


// CPU initialization - PLL, Clock, Timers, GPIO
void CPU_Init()// Initialization of CPU: PLL, Clock, Timers, GPIO
{
    EALLOW;
    // Init System Control - CPU: PLL, Clocks to Peripherals. 90 MHz from internal oscillator
    InitSysCtrl();      //PLLCR = 18, DIVSEL = 2 -> SYSCLOCKOUT = 10x18/2 = 90 MHz, LOSPCP = 2 -> LSPCLK = SYSCLKOUT/4 = 44 ns

    // Initialize timers
    InitCpuTimers();
    ConfigCpuTimer(&CpuTimer0, fCPU, Timer0Period); //10 kHz - Fast Calc
    ConfigCpuTimer(&CpuTimer1, fCPU, Timer1Period); //1 kHz - kHz Calc
    CpuTimer0Regs.TCR.all = 0x4000; //Start CPU Timer 0
    CpuTimer1Regs.TCR.all = 0x4000; //Start CPU Timer 1
    EDIS;

    //Watchdog
    DisableDog();
}

//====================================================
// Variables for testing purposes
int z_test = 0; //
int pwm_on_test = 0; //Switches on PWM on all 3 phases
int Adc_test_cnt = 0;
long Adc_test_avg, Adc_test_sum = 0;

// Variables related to Booster Board HW
int EN_GATE, DRV_FAULT = 0;

//====================================================
//Global variables
Uint16 fCPU = 90;    //CPU Frequency, MHz
Uint16 fPWM = 10;     //PWM Frequency, kHz
int Timer0Period = 100;
int Timer1Period = 1000;
unsigned int SpiOutData = 0x0;
unsigned int SpiInData = 0x0;
unsigned short Sys_Sync_Flag = 0;

//====================================================
//Main variables-instances of program modules

TSM_Sys sm_sys  = SM_SYS_DEFAULTS;          // System management variable, includes system init, Fast Calc on PWM freq, Slow Calc
TSM_Ctrl ctrl   = SM_CTRL_DEFAULTS;         // Control system variable, main state machine
TDrv_Param drv_param = DRV_PARAM_DEFAULTS;  // Parameters of  the drive - PU are used across the whole project
TAdc    adc     = ADC_DEFAULTS;             // ADC init and processing
TPwm    pwm     = PWM_DEFAULTS;             // Pwm
TCAN    can     = CAN_DEFAULTS;             // CAN
TQep    qep     = QEP_DEFAULTS;             // QEP - encoder
TClarke clarke  = CLARKE_DEFAULTS;          // Clarke transform
TPark   park    = PARK_DEFAULTS;            // Park transform
TIPark  ipark   = IPARK_DEFAULTS;           // Inverse Park transform
TRmp    rmp     = RMP_DEFAULTS;             // Ramp generator
TRefs   refs    = REFS_DEFAULTS;            // Main refs of the drive
TCW     cw      = CW_DEFAULTS;              // Main control word
TSW     sw      = SW_DEFAULTS;              // Main status word
TCurPar curpar  = CUR_PAR_DEFAULTS;        // Current parameters of the control system
TPid    pid_iq  = PID_DEFAULTS;             // PID regulator for iq - torque-generating current
TPid    pid_id  = PID_DEFAULTS;             // PID regulator for id - magnetizing current
TPid    pid_speed  = PID_DEFAULTS;          // PID regulator for speed
TProt   prot    = PROT_DEFAULTS;            // Protections

//====================================================
// Aux variables
Uint16 LoopCounter = 0;             // Counter of loops of main

//====================================================
// Aux variables - PWM Trip Zone counters
int Epwm1_tzint_cnt, Epwm2_tzint_cnt = 0;

//====================================================
// Main
//
void main(void)
{

    unsigned long i;

    //====================================================
    // If running from flash copy RAM only functions to RAM
    DINT;
#ifdef _FLASH
    memcpy(&RamfuncsRunStart, &RamfuncsLoadStart, (size_t)&RamfuncsLoadSize);
#endif      

    //====================================================
    //Initialize CPU - PLL, Clock, Timers
    CPU_Init();

    //PWM force off - just to be safe
    pwm.off(&pwm);

    //Initialize System Control
    sm_sys.init(&sm_sys);

    //Enable interrupts
    EINT;

    //====================================================
    // +++ Main program loop +++
    for(;;)
    {
        LoopCounter++;
        sm_sys.slow_calc(&sm_sys); //background calculation, that are not related to strict real-time domain
    }
}

//====================================================================
//Main interrupt handlers - for timer0 at PWM frequency, for Timer 1 at fixed 1 kHz, and a TZ Interrupt

Uint32 CpuTimeCnt10, CpuTimeCnt1 = 0;   //Variable for current "time" in CPU clocks
Uint32 TIsr10, TIsr1 = 0;               //"Time" of a function, in CPU clocks, derived as a diff between start and stop CPU cnts
Uint32 CounterTime10, CounterTime1 = 0; //CPU cnts

//Main Interrupt at PWM freq - PWM-based interrupt is used, EPWM4 INT
__interrupt void TI10_IRQ_Handler(void)
{
    if (Sys_Sync_Flag == 0){
        while (CpuTimer0Regs.TIM.all < 8980){
            DSP28x_usDelay(1);
        }
        Sys_Sync_Flag = 1; //system synchronized
    }

    CpuTimeCnt10 = CpuTimer0Regs.TIM.all;   //saving current "time"

    //-------------------------
    sm_sys.fast_calc(&sm_sys); // Control System fast calc
    //-------------------------

    CounterTime10++;

    //TIsr10 = (CpuTimeCnt10 - CpuTimer0Regs.TIM.all) & 0xFFFFFF; //cutting off everything over 1 cycle
    //if (TIsr10 > (Timer0Period - 5) * fCPU){            //100 - 5 = 95 - check for too long program execution
        //sm_prot.fault_word1 |= FAULT_PROGRAM10;       // Fault - too long program execution
    //}

    EPwm4Regs.ETCLR.all = 0x1;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;
}

//Main Interrupt at fixed 1 kHz - needed for non-pwm related tasks, but those that require fixed freq of call
__interrupt void TI1_IRQ_Handler(void)
{
    CpuTimeCnt1 = CpuTimer1Regs.TIM.all;   //saving current "time"

    //-------------------------
    sm_sys.khz_calc(&sm_sys); //fix calc at 1 kHz
    //-------------------------

    CounterTime1++;

    TIsr1 = (CpuTimeCnt1 - CpuTimer1Regs.TIM.all) & 0xFFFFFF; //calculating time of execution
    if (TIsr1 > (Timer1Period - 50) * fCPU){        //1000 - 50 = 950 - check for too long program execution
        //sm_prot.fault_word1 |= FAULT_PROGRAM1;    //Fault - too long program execution
    }

    CpuTimer1Regs.TCR.bit.TIF = 0;          //TIM1 INT flag clear
}

// Конец и вновь начало...
