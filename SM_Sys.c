//
//Main count of Control System - Init, Fast Calc, Slow Calc
//

#include "V_Include/SM_Sys.h"
#include "V_Include/main.h"

// Control System init - program modules, INTerrupts
void SM_Sys_Init(TSM_Sys *p)
{
    //====================================================
     // INT init

     InitPieCtrl();
     InitPieVectTable();

     IER = 0x0000; // Disable CPU interrupts and clear all CPU interrupt flags:
     IFR = 0x0000;

     EALLOW;
     PieVectTable.ADCINT1       = &ADCINT1_Handler;    //ADC INT for tests
     PieVectTable.EPWM4_INT     = &TI10_IRQ_Handler;   //10 kHz Fast Calc IRQ
     PieVectTable.TINT1         = &TI1_IRQ_Handler;   //1 kHz Slow Calc IRQ
     PieVectTable.EPWM1_TZINT   = &EPWM1_TZINT_Handler;   //TZ interrupt EPWM1, group 2
     PieVectTable.EPWM2_TZINT   = &EPWM2_TZINT_Handler;   //TZ interrupt EPWM2, group 2
     EDIS;

     //PieCtrlRegs.PIEIER1.bit.INTx1 = 1; //Enable ADCINT1 - INT of test ADC
     PieCtrlRegs.PIEIER2.bit.INTx1 = 1; //Enable EPWM1_TZINT
     PieCtrlRegs.PIEIER2.bit.INTx2 = 1; //Enable EPWM2_TZINT
     PieCtrlRegs.PIEIER3.bit.INTx4 = 1; //Enable EPWM4 INT - Main interrupt of Control System calc + common ADC (Udc etc.)

     //IER |= M_INT1; //Enable Group 1 Interrupts - ADCINT1
     IER |= M_INT2; //Enable Group 2 Interrupts - EPWM TZ
     IER |= M_INT3; //Enable Group 3 Interrupts - EPWM4
     IER |= M_INT13; //Enable Group 13 Interrupts - TINT1

     EINT;   // Enable Global interrupt INTM
     ERTM;   // Enable Global realtime interrupt DBGM

     //====================================================
     // GPIO
     // Initialize GPIOs for the LEDs and turn them off
     EALLOW;
     GpioCtrlRegs.GPBDIR.bit.GPIO34 = 1;
     GpioCtrlRegs.GPBDIR.bit.GPIO39 = 1;
     GpioDataRegs.GPBDAT.bit.GPIO34 = 1;
     GpioDataRegs.GPBDAT.bit.GPIO39 = 1;

     GpioCtrlRegs.GPAPUD.bit.GPIO0  = 1;    // EPWM Pull-up off
     GpioCtrlRegs.GPAPUD.bit.GPIO1  = 1;    // EPWM Pull-up off
     GpioCtrlRegs.GPAPUD.bit.GPIO2  = 1;    // EPWM Pull-up off
     GpioCtrlRegs.GPAPUD.bit.GPIO3  = 1;    // EPWM Pull-up off
     GpioCtrlRegs.GPAPUD.bit.GPIO4  = 1;    // EPWM Pull-up off
     GpioCtrlRegs.GPAPUD.bit.GPIO5  = 1;    // EPWM Pull-up off
     GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 1;    // 0=GPIO,  1=EPWM1A
     GpioCtrlRegs.GPAMUX1.bit.GPIO1 = 1;    // 0=GPIO,  1=EPWM1B
     GpioCtrlRegs.GPAMUX1.bit.GPIO2 = 1;    // 0=GPIO,  1=EPWM2A
     GpioCtrlRegs.GPAMUX1.bit.GPIO3 = 1;    // 0=GPIO,  1=EPWM2B
     GpioCtrlRegs.GPAMUX1.bit.GPIO4 = 1;    // 0=GPIO,  1=EPWM3A
     GpioCtrlRegs.GPAMUX1.bit.GPIO5 = 1;    // 0=GPIO,  1=EPWM3B


     //----------------
     //GPIO для Booster board

     // Fault and Overcurrent IOs - pullup needed for booster's open drain transistor
     GpioCtrlRegs.GPAPUD.bit.GPIO28 = 0;    //Pull Up ON
     GpioCtrlRegs.GPAPUD.bit.GPIO29 = 0;    //Pull Up ON
     GpioCtrlRegs.GPAMUX2.bit.GPIO28 = 0;   // 0=GPIO
     GpioCtrlRegs.GPAMUX2.bit.GPIO29 = 0;   // 0=GPIO
     GpioCtrlRegs.GPADIR.bit.GPIO28 = 0;    // 0=Input
     GpioCtrlRegs.GPADIR.bit.GPIO29 = 0;    // 0=Input
     GpioDataRegs.GPADAT.bit.GPIO28 = 0;
     GpioDataRegs.GPADAT.bit.GPIO29 = 0;

     //EN_GATE - to start gate drivers an physically enable PWM
     GpioCtrlRegs.GPBPUD.bit.GPIO50 = 1;    //Pull Up OFF
     GpioCtrlRegs.GPBMUX2.bit.GPIO50 = 0;   // 0=GPIO
     GpioCtrlRegs.GPBDIR.bit.GPIO50 = 1;    // 1 = Output
     GpioDataRegs.GPBDAT.bit.GPIO50 = 0;    // Disable Gate by default
     EDIS;

     //=====================================================
    //Initialize ADC
    InitAdc();      //HW init of ADC
    adc.init(&adc); //System init of ADC
    //Adc_init(); //User init of ADC - Udc, Ua, Ub, Uc, Ia, Ib, (Temperature)

    //=====================================================
    //Initialize ePWM
    pwm.init(&pwm);

    //=====================================================
    //Init of everything else
    drv_param.init(&drv_param);
}

void SM_Sys_Fast_Calc(TSM_Sys *p) // Fast calc on PWM frequency
{
    adc.fast_calc(&adc);
    pwm.fast_calc(&pwm);
}

void SM_Sys_Khz_Calc(TSM_Sys *p) // Calc at fixed frequency of 1 kHz
{
    //All test functions
    TestFuncs();
}

void SM_Sys_Slow_Calc(TSM_Sys *p) // Off-real-time background calc
{
    adc.slow_calc(&adc);
    drv_param.slow_calc(&drv_param);
}
