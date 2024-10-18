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
#include <string.h>             // Г¤Г«Гї memcopy


// CPU initialization - PLL, Clock, Timers, GPIO
void CPU_Init()// Initialization of CPU: PLL, Clock, Timers, GPIO
{
    EALLOW;
    // Init System Control - CPU: PLL, Clocks to Peripherals. 90 MHz from internal oscillator
    InitSysCtrl();      //PLLCR = 18, DIVSEL = 2 -> SYSCLOCKOUT = 10x18/2 = 90 MHz

    // Initialize timers
    InitCpuTimers();
    //ConfigCpuTimer(&CpuTimer0, fCPU, Timer0Period); //10 kHz - Fast Calc
    ConfigCpuTimer(&CpuTimer1, fCPU, Timer1Period); //1 kHz - kHz Calc
    //CpuTimer0Regs.TCR.all = 0x4000; //Start CPU Timer 0
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
int Timer0Period = 1 * 1000;
int Timer1Period = 1000;

//====================================================
//Main variables-instances of program modules
TSM_Sys sm_sys = SM_SYS_DEFAULTS;   // Control System variable, includes system init, Fast Calc on PWM freq, Slow Calc
TAdc adc = ADC_DEFAULTS;            // ADC init and processing
TDrv_Param drv_param = DRV_PARAM_DEFAULTS; // Parameters of  the drive - PU are used across the whole project
TPwm pwm = PWM_DEFAULTS;            // Pwm
//TClarke clarke = CLARKE_DEFAULTS;   // Clarke transform
//TPark park = PARK_DEFAULTS;         // Park transform

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
        sm_sys.slow_calc(&sm_sys); //bacground calculation, that are not related to strict real-time domain

        //Test function for manual PWM switch on and off
        if (pwm_on_test == 1) {
            pwm.on(&pwm);
            pwm_on_test = 0;
        } else if (pwm_on_test == 2){
            pwm.off(&pwm);
            pwm_on_test = 0;
        }
    }
}

//====================================================================
//Main interrupt handlers - for timer0 at PWM frequency, for Timer 1 at fixed 1 kHz, and a TZ Interrupt

Uint32 CpuTimeCnt10, CpuTimeCnt1 = 0;   //Variable for current "time" in CPU clocks
Uint32 TIsr10, TIsr1 = 0;               //"Time" of a function, in CPU clocks, derived as a diff between start and stop CPU cnts
Uint32 CounterTime10, CounterTime1 = 0; //CPU cnts

//Main Interrupt at PWM freq - not used right now: PWM-based interrupt is used instead
__interrupt void TI10_IRQ_Handler(void)
{
    CpuTimeCnt10 = CpuTimer0Regs.TIM.all;   //saving current "time"

    //-------------------------
    sm_sys.fast_calc(&sm_sys); // Control System fast calc
    //-------------------------

    CounterTime10++;

    TIsr10 = (CpuTimeCnt10 - CpuTimer0Regs.TIM.all) & 0xFFFFFF; //cutting off everything over 1 cycle
    if (TIsr10 > (Timer0Period - 5) * fCPU){            //100 - 5 = 95 - check for too long program execution
        //sm_prot.fault_word1 |= FAULT_PROGRAM10;       // Fault - too long program execution
    }

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

//Trip Zone interrupt - based on Ia value
__interrupt void EPWM1_TZINT_Handler(void)
{
    pwm.off(&pwm);

    Epwm1_tzint_cnt++;
    //EPwm1Regs.TZCLR.all             = 0xF;                    //All TZ flag clear
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP2;
}

//Trip Zone interrupt - based on Ib value
__interrupt void EPWM2_TZINT_Handler(void)
{
    pwm.off(&pwm);

    Epwm2_tzint_cnt++;
    //EPwm2Regs.TZCLR.all             = 0xF;                    //All TZ flag clear
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP2;
}

// End and beginning again...
