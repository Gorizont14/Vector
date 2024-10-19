//
// main.h - Common header file linking all other headers and .c files together
//

#ifndef MAIN_H
#define MAIN_H

#include "stdlib.h"

#include "DSP28x_Project.h"     // DSP28x Headerfile
#include "F2806x_Device.h"

//#include "IQmath.h"
#include "sci_io.h"
#include "SM_Sys.h"
#include "V_Adc.h"
#include "V_Drv_Param.h"
#include "V_Pwm.h"

extern TSM_Sys      sm_sys;
extern TAdc         adc;
extern TDrv_Param   drv_param;
extern TPwm         pwm;

extern int          Epwm1_tzint_cnt, Epwm2_tzint_cnt;
extern int          Adc_test_cnt;
extern long         Adc_test_avg, Adc_test_sum;

extern Uint16 fCPU;         //CPU freq, MHz
extern Uint16 fPWM;         //PWM freq, kHz
extern int Timer0Period;    //Period of Timer 0 - PWM timer, in us
extern int Timer1Period;    //Period of Timer 0 - 1 kHz fixed freq timer, in us

extern Uint16 LoopCounter;  //counter of main cycles, background calc
extern Uint32 CpuTimeCnt10, CpuTimeCnt1;   //Current value of timers 1 and 0
extern Uint32 TIsr10, TIsr1;               //Time of calc of INTs 10 and 1 kHz, in CPU ticks
extern Uint32 CounterTime10, CounterTime1; //Cnt of INTs 1 and 10 kHz

//Aux variables
extern int DRV_FAULT;
extern int EN_GATE;

// Interrupt Handlers declaration
__interrupt void TI10_IRQ_Handler(void);
__interrupt void TI1_IRQ_Handler(void);
__interrupt void EPWM1_TZINT_Handler(void);
__interrupt void EPWM2_TZINT_Handler(void);
__interrupt void ADCINT1_Handler(void);

// Aux test functions
extern void TestFuncs(void);

#ifndef TRUE
#define TRUE  (1)
#endif
#ifndef FALSE
#define FALSE (0)
#endif

#endif /* MAIN_H_ */
