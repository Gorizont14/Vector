//###########################################################################
//
// FILE:	My main - �������� ���� � main-��
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
#include <string.h>             // ��� memcopy


// ������� ������������ ���������������� - PLL, Clock, Timers, GPIO
void CPU_Init()// ������� ������������ ���������������� - PLL, Clock, Timers, GPIO
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
// ��������� ����������
int z_test = 0; // ��������� ���������� ��� �������
int pwm_on_test = 0; //������ ���������� pwm
int Adc_test_cnt = 0;
long Adc_test_avg, Adc_test_sum = 0;

// ��������� ���������� ��� Booster Board
int EN_GATE, DRV_FAULT = 0;

//====================================================
//Global variables
Uint16 fCPU = 90;    //������� CPU � ���
Uint16 fPWM = 10;     //������� ��� � ���
int Timer0Period = 1 * 1000; //������ ������� 0 - ������� ��� - ������� ���, � ���
int Timer1Period = 1000; //������ ������� 1 - ������� �������������� ������� 1 �Hz, � ���

//====================================================
// ����������� ������
TSM_Sys sm_sys = SM_SYS_DEFAULTS;   // �������� ������ ���� �������, ����� ���� ����������� ������� in Fast Calc, Slow Calc
TAdc adc = ADC_DEFAULTS;            // ���
TDrv_Param drv_param = DRV_PARAM_DEFAULTS; //����������� �������� ������� � ���������
TPwm pwm = PWM_DEFAULTS;            //Pwm
//TClarke clarke = CLARKE_DEFAULTS;   // ������ ��������������
//TPark park = PARK_DEFAULTS;         // ������������ ��������������

//====================================================
//���������� ����������
Uint16 LoopCounter = 0;             // ������� �������� ����� main-�

//====================================================
// ��������� ����������
// ������� ����������
int Epwm1_tzint_cnt, Epwm2_tzint_cnt = 0;
//====================================================
// Main
//
void main(void)
{
    // ������� ��������� ����������
    unsigned long i;

    //====================================================
    // If running from flash copy RAM only functions to RAM

    // ������ ����������
    DINT;
#ifdef _FLASH
    memcpy(&RamfuncsRunStart, &RamfuncsLoadStart, (size_t)&RamfuncsLoadSize);
#endif      

    //====================================================
    //Initialize CPU - PLL, Clock, Timers
    CPU_Init();

    //��������� ��� (�� ������ ������)
    pwm.off(&pwm);

    //Initialize System Control - ������������� ����������, ADC, PWM, ����������� ������� - �� ���� ����� ��� ���� ����� hardware ������ ��
    sm_sys.init(&sm_sys);

    //���������� ����������
    EINT;

    //====================================================
    // +++ Main program loop +++
    for(;;)
    {
        LoopCounter++;
        sm_sys.slow_calc(&sm_sys); //������� ������

        //���� �������� ���
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
//��������� ���������� Interrupt IRQ Hanlder-��

Uint32 CpuTimeCnt10, CpuTimeCnt1 = 0;   //������� �������� �������� �������� 0 � 1
Uint32 TIsr10, TIsr1 = 0;               //����� ������� ���������� 10 � 1 ���, � ������ ����������
Uint32 CounterTime10, CounterTime1 = 0; //������� ������ ���������� 10 � 1 ���

//���������� 10 ��� - �� ������� ���, ������������ EPWM4
__interrupt void TI10_IRQ_Handler(void)
{
    CpuTimeCnt10 = CpuTimer0Regs.TIM.all;   //�������� ������� �����

    //-------------------------
    sm_sys.fast_calc(&sm_sys); //fast calc �� ������� �10 ������� ���
    //-------------------------

    CounterTime10++;

    TIsr10 = (CpuTimeCnt10 - CpuTimer0Regs.TIM.all) & 0xFFFFFF; //������� ����� � ������
    if (TIsr10 > (Timer0Period - 5) * fCPU){          //100 - 5 = 95 ��� * 90 ��� = 94 ���
        //sm_prot.fault_word1 |= FAULT_PROGRAM10;
    }

    EPwm4Regs.ETCLR.all = 0x1;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;
}

//���������� 1 ���
__interrupt void TI1_IRQ_Handler(void)
{
    CpuTimeCnt1 = CpuTimer1Regs.TIM.all;   //�������� ������� �����

    //-------------------------
    sm_sys.khz_calc(&sm_sys); //fix calc �� ������� 1 kHz
    //-------------------------

    CounterTime1++;

    TIsr1 = (CpuTimeCnt1 - CpuTimer1Regs.TIM.all) & 0xFFFFFF; //������� ����� � ������
    if (TIsr1 > (Timer1Period - 50) * fCPU){    //1000 - 50 = 950 ��� * 90 ��� = 940 ���
        //sm_prot.fault_word1 |= FAULT_PROGRAM1;
    }

    CpuTimer1Regs.TCR.bit.TIF = 0;          //����� ����� ���������� TIM1
}

//���������� ���������� ��������� - ������ ���
__interrupt void EPWM1_TZINT_Handler(void)
{
    //��������� ���
    pwm.off(&pwm);

    Epwm1_tzint_cnt++;
    //EPwm1Regs.TZCLR.all             = 0xF;                    //All TZ flag clear
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP2;
}

__interrupt void EPWM2_TZINT_Handler(void)
{
    //��������� ���
    pwm.off(&pwm);

    Epwm2_tzint_cnt++;
    //EPwm2Regs.TZCLR.all             = 0xF;                    //All TZ flag clear
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP2;
}

// ����� � ����� ������...
