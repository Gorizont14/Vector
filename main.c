//###########################################################################
//
// FILE:	My main - Основной файл с main-ом
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
#include <string.h>             // для memcopy


// Функция иницилазации микроконтроллера - PLL, Clock, Timers, GPIO
void CPU_Init()// Функция иницилазации микроконтроллера - PLL, Clock, Timers, GPIO
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

    //Timer0Period = fPWM * 1000;
}

//====================================================
// Служебные переменные
int z_test = 0; // служебная переменная для отладки
int pwm_on_test = 0; //ручной включатель pwm
int Adc_test_cnt = 0;
long Adc_test_avg, Adc_test_sum = 0;

// Служебные переменные для Booster Board
int EN_GATE, DRV_FAULT = 0;

//====================================================
//Global variables
Uint16 fCPU = 90;    //частота CPU в МГц
Uint16 fPWM = 10;     //Частота ШИМ в кГц
int Timer0Period = 1 * 1000; //период таймера 0 - таймера ШИМ - частота ШИМ, в мкс
int Timer1Period = 1000; //период таймера 1 - таймера фиксированного расчета 1 кHz, в мкс

//====================================================
// Программные модули
TSM_Sys sm_sys = SM_SYS_DEFAULTS;   // Основной расчет всей системы, вызов всех программных модулей in Fast Calc, Slow Calc
TAdc adc = ADC_DEFAULTS;            // АЦП
TDrv_Param drv_param = DRV_PARAM_DEFAULTS; //Номинальные значения привода и двигателя
TPwm pwm = PWM_DEFAULTS;            //Pwm
//TClarke clarke = CLARKE_DEFAULTS;   // Фазные преобразования
//TPark park = PARK_DEFAULTS;         // Координатные преобразования

//====================================================
//Глобальные переменные
Uint16 LoopCounter = 0;             // Счетчик фонового цикла main-а

//====================================================
// Служебные переменные
// Маркеры прерываний
int Epwm1_tzint_cnt, Epwm2_tzint_cnt = 0;
//====================================================
// Main
//
void main(void)
{
    // Парочка служебных переменных
    unsigned long i;

    //====================================================
    // If running from flash copy RAM only functions to RAM

    // запрет прерываний
    DINT;
#ifdef _FLASH
    memcpy(&RamfuncsRunStart, &RamfuncsLoadStart, (size_t)&RamfuncsLoadSize);
#endif      

    //====================================================
    //Initialize CPU - PLL, Clock, Timers
    CPU_Init();

    //Выключаем ШИМ (на всякий случай)
    pwm.off(&pwm);

    //Initialize System Control - инициализация прерываний, ADC, PWM, программных модулей - по сути всего что есть кроме hardware самого МК
    sm_sys.init(&sm_sys);

    //Разрешение прерываний
    EINT;

    //====================================================
    // +++ Main program loop +++
    for(;;)
    {
        LoopCounter++;
        sm_sys.slow_calc(&sm_sys); //фоновый расчет

        //ТЕСТ Включаем ШИМ
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
//Служебные переменные Interrupt IRQ Hanlder-ов

Uint32 CpuTimeCnt10, CpuTimeCnt1 = 0;   //текущее значение счетчика таймеров 0 и 1
Uint32 TIsr10, TIsr1 = 0;               //время расчета прерывания 10 и 1 кГц, в тактах процессора
Uint32 CounterTime10, CounterTime1 = 0; //счетчик циклов прерываний 10 и 1 кГц

//Прерывание 10 кГц - на частоте ШИМ, инициируется EPWM4
__interrupt void TI10_IRQ_Handler(void)
{
    CpuTimeCnt10 = CpuTimer0Regs.TIM.all;   //засекаем текущее время

    //-------------------------
    sm_sys.fast_calc(&sm_sys); //fast calc на частоте х10 частоты ШИМ
    //-------------------------

    CounterTime10++;

    TIsr10 = (CpuTimeCnt10 - CpuTimer0Regs.TIM.all) & 0xFFFFFF; //считаем время в тактах
    if (TIsr10 > (Timer0Period - 5) * fCPU){          //100 - 5 = 95 мкс * 90 МГц = 94 мкс
        //sm_prot.fault_word1 |= FAULT_PROGRAM10;
    }

    EPwm4Regs.ETCLR.all = 0x1;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;
}

//Прерывание 1 кГц
__interrupt void TI1_IRQ_Handler(void)
{
    CpuTimeCnt1 = CpuTimer1Regs.TIM.all;   //засекаем текущее время

    //-------------------------
    sm_sys.khz_calc(&sm_sys); //fix calc на частоте 1 kHz
    //-------------------------

    CounterTime1++;

    TIsr1 = (CpuTimeCnt1 - CpuTimer1Regs.TIM.all) & 0xFFFFFF; //считаем время в тактах
    if (TIsr1 > (Timer1Period - 50) * fCPU){    //1000 - 50 = 950 мкс * 90 МГц = 940 мкс
        //sm_prot.fault_word1 |= FAULT_PROGRAM1;
    }

    CpuTimer1Regs.TCR.bit.TIF = 0;          //сброс флага прерывания TIM1
}

//Аппаратное прерывание инвертора - авария ШИМ
__interrupt void EPWM1_TZINT_Handler(void)
{
    //Выключаем ШИМ
    pwm.off(&pwm);

    Epwm1_tzint_cnt++;
    //EPwm1Regs.TZCLR.all             = 0xF;                    //All TZ flag clear
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP2;
}

__interrupt void EPWM2_TZINT_Handler(void)
{
    //Выключаем ШИМ
    pwm.off(&pwm);

    Epwm2_tzint_cnt++;
    //EPwm2Regs.TZCLR.all             = 0xF;                    //All TZ flag clear
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP2;
}

// Конец и вновь начало...
