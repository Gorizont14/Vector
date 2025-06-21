//
//File with functions for testing
//

#include "V_Include/main.h"

long test_clock = 0;
int test_buttons = 0;
//0 - SPI 1 transmittion init
float   Speed_Max_High = 0.0;
float   Speed_Max_Low = 0.0;

void AdcAvg(void)
{
    Adc_test_sum += AdcResult.ADCRESULT7;
    Adc_test_cnt++;
    if (Adc_test_cnt > 20)
    {
        Adc_test_avg = Adc_test_sum/Adc_test_cnt;
        Adc_test_sum = 0;
        Adc_test_cnt = 0;
    }
}

__interrupt void ADCINT1_Handler(void)
{
    AdcAvg();
    AdcRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

void TestFuncs(void)
{
    AdcAvg();
    if(test_buttons & 0x0001){
        SpiServ();
    }

    //Test function for manual PWM switch on and off
    if (pwm_on_test == 1) {
        pwm.on(&pwm);
        pwm_on_test = 0;
    } else if (pwm_on_test == 2){
        pwm.off(&pwm);
        pwm_on_test = 0;
    }
}
