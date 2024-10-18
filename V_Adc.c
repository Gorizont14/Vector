//
//ADC Calc
//

#include "V_Include/V_Adc.h"
#include "V_Include/main.h"

#define ADC_SCALE 1/2048.0

void Adc_Init(TAdc *p)
{
        //ADC Pins
        //Udc   ADCINA7
        //Ua    ADCINB1
        //Ub    ADCINA2
        //Uc    ADCINB2
        //Ia    ADCINA0
        //Ib    ADCINB0
        //Ic    ADCINA1
        //TEST  ADCINB7

        EALLOW;

        //Clearing of ADC soc flags, force, overflow flags, interrupt flags
        AdcRegs.ADCSOCFLG1.all      = 0;
        AdcRegs.ADCSOCFRC1.all      = 0;
        AdcRegs.ADCSOCOVFCLR1.all  |= 0x7F;  //SOC  Overflow 1 - 7
        AdcRegs.ADCINTFLGCLR.all   |= 0x7F;  //ADCINT 1 - 7
        AdcRegs.ADCINTOVFCLR.all   |= 0x7F;  //ADC INT Overflow 1 - 7

        //Clearing ADC result buffers
        AdcResult.ADCRESULT0 = 0;
        AdcResult.ADCRESULT1 = 0;
        AdcResult.ADCRESULT2 = 0;
        AdcResult.ADCRESULT3 = 0;
        AdcResult.ADCRESULT4 = 0;
        AdcResult.ADCRESULT5 = 0;
        AdcResult.ADCRESULT6 = 0;
        AdcResult.ADCRESULT7 = 0;   //Test

        AdcRegs.ADCCTL2.bit.ADCNONOVERLAP   = 0;
        AdcRegs.ADCCTL1.bit.INTPULSEPOS     = 1;    //1 cycle prior to EOC

        AdcRegs.ADCSOC0CTL.bit.TRIGSEL      = 0xB;  // ePWM4SOCA
        AdcRegs.ADCSOC1CTL.bit.TRIGSEL      = 0x5;  // ePWM1SOCA
        AdcRegs.ADCSOC2CTL.bit.TRIGSEL      = 0x7;  // ePWM2SOCA
        AdcRegs.ADCSOC3CTL.bit.TRIGSEL      = 0x9;  // ePWM3SOCA
        AdcRegs.ADCSOC4CTL.bit.TRIGSEL      = 0x5;  // ePWM1SOCA
        AdcRegs.ADCSOC5CTL.bit.TRIGSEL      = 0x7;  // ePWM2SOCA
        AdcRegs.ADCSOC6CTL.bit.TRIGSEL      = 0x9;  // ePWM3SOCA
        AdcRegs.ADCSOC7CTL.bit.TRIGSEL      = 0xD;  // ePWM5SOCA

        AdcRegs.ADCINTSOCSEL1.all           = 0;    // 0 - no ADCINT triggers SOC 0-7
        AdcRegs.ADCINTSOCSEL2.all           = 0;    // 0 - no ADCINT triggers SOC 8-15

        AdcRegs.ADCSOC0CTL.bit.CHSEL        = 0x7;  //ADCINA7   Udc
        AdcRegs.ADCSOC1CTL.bit.CHSEL        = 0x9;  //ADCINB1   Ua
        AdcRegs.ADCSOC2CTL.bit.CHSEL        = 0x2;  //ADCINA2   Ub
        AdcRegs.ADCSOC3CTL.bit.CHSEL        = 0xA;  //ADCINB2   Uc
        AdcRegs.ADCSOC4CTL.bit.CHSEL        = 0x0;  //ADCINA0   Ia
        AdcRegs.ADCSOC5CTL.bit.CHSEL        = 0x8;  //ADCINB0   Ib
        AdcRegs.ADCSOC6CTL.bit.CHSEL        = 0x1;  //ADCINA1   Ic
        AdcRegs.ADCSOC7CTL.bit.CHSEL        = 0xF;  //ADCINB7   Test

        AdcRegs.ADCSOC0CTL.bit.ACQPS        = 9;    // cycles per sample, 9 = 100ns = 0.1us at 90MHz
        AdcRegs.ADCSOC1CTL.bit.ACQPS        = 9;    // cycles per sample
        AdcRegs.ADCSOC2CTL.bit.ACQPS        = 9;    // cycles per sample
        AdcRegs.ADCSOC3CTL.bit.ACQPS        = 9;    // cycles per sample
        AdcRegs.ADCSOC4CTL.bit.ACQPS        = 9;    // cycles per sample
        AdcRegs.ADCSOC5CTL.bit.ACQPS        = 9;    // cycles per sample
        AdcRegs.ADCSOC6CTL.bit.ACQPS        = 9;    // cycles per sample
        AdcRegs.ADCSOC7CTL.bit.ACQPS        = 9;    // cycles per sample

        AdcRegs.ADCSAMPLEMODE.all           = 0;    // 1 - SOC0+1 simultaniously, 0 - All SOCs are single
        AdcRegs.SOCPRICTL.bit.SOCPRIORITY   = 0;    // 1 - SOC0 Priority, 0 - Round Robin for all

        AdcRegs.INTSEL1N2.bit.INT1SEL       = 0x7;    //EOC 7 triggers ADCINT 1
        AdcRegs.INTSEL1N2.bit.INT1E         = 1;   // Enable ADCINT 1
        AdcRegs.INTSEL1N2.bit.INT1CONT      = 1;    //Continious INT generation - not needed
        EDIS;
}

void Adc_Fast_calc(TAdc *p)
{
    // Main ADC Calc on PWM freq
    p->Itemp_a = 0;
    p->Itemp_b = 0;
    p->Udctemp = 0;

    p->Itemp_a = AdcResult.ADCRESULT4; //0-4095
    p->Itemp_a -= 2048; //+-2048
    p->Imeas_a = p->IaGainNom * (p->Itemp_a * ADC_SCALE + p->Imeas_a_offset); //32768 x 10, int32

    //--------------------------------------------
    //TEST - temporary TZ of phase A
    if(abs(p->Imeas_a) > 0.5 && EN_GATE == 1)
    {
        //pwm.off(&pwm);
    }
    //--------------------------------------------

    p->Itemp_b = AdcResult.ADCRESULT5; //0-4095
    p->Itemp_b -= 2048; //+-2048
    p->Imeas_b = p->IbGainNom * (p->Itemp_b * ADC_SCALE + p->Imeas_b_offset);

    p->Imeas_c = -p->Imeas_a - p->Imeas_b; //Ic = -Ia - Ib

    p->Udctemp = AdcResult.ADCRESULT0; //0-4095
    p->Udctemp -= 2048; //+-2048
    p->Umeas_dc = p->UdcGainNom * (p->Udctemp * ADC_SCALE + p->Umeas_dc_offset); //
}

void Adc_Khz_calc(TAdc *p)
{

}

void Adc_Slow_calc(TAdc *p)
{
    //Recalc of nominal coefficients of current, voltage etc.
    p->IaGainNom = p->Imeas_a_gain / drv_param.I_nom;
    p->IbGainNom = p->Imeas_b_gain / drv_param.I_nom;
    p->UdcGainNom = p->Umeas_dc_gain / drv_param.Udc_nom;
}
