//
//File with functions for testing
//

#include "V_Include/main.h"

long test_clock;

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

void FaultCheck(void)
{
    if (GpioDataRegs.GPADAT.bit.GPIO28 == 0) {
        DRV_FAULT = 1;
        test_clock++;
    } else {
        DRV_FAULT = 0;
    }

}

void TestFuncs(void)
{
    AdcAvg();
    FaultCheck();
}
