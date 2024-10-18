//
//PWM Calc
//

#include "V_Include/V_Pwm.h"
#include "V_Include/main.h"

void Pwm_Init(TPwm *p) //Main calc of 3-phase PWM, Control System call and ADC call
{
    EALLOW;
        SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 0; //Switch off PWM clock
        SysCtrlRegs.PCLKCR1.bit.EPWM1ENCLK = 1;//Enable all PWMs: 1,2,3 for phases A,B,C, 4 for Control System + common things like ADC Udc, 5 for testing
        SysCtrlRegs.PCLKCR1.bit.EPWM2ENCLK = 1;
        SysCtrlRegs.PCLKCR1.bit.EPWM3ENCLK = 1;
        SysCtrlRegs.PCLKCR1.bit.EPWM4ENCLK = 1;
        SysCtrlRegs.PCLKCR1.bit.EPWM5ENCLK = 1;
    EDIS;

    //====================================================
    //EPWM1
    //--------------------------------------------------------------------------------
    EPwm1Regs.TBPRD                     = 45000000 / p->Pwm_freq;   // Period - 11250 for 4KHz, 4500 for 10 kHz
    EPwm1Regs.TBPHS.half.TBPHS          = 0;                        // Phase
    EPwm1Regs.TBCTR                     = 0;                        // zero to cnt

    EPwm1Regs.CMPA.half.CMPA            = EPwm1Regs.TBPRD + 1;      // Set CTR level
    //--------------------------------------------------------------------------------

    EPwm1Regs.TBCTL.bit.CTRMODE         = TB_COUNT_UPDOWN;  // Symmetrical mode
    EPwm1Regs.TBCTL.bit.PHSEN           = TB_DISABLE;       // Phase control
    EPwm1Regs.TBCTL.bit.PRDLD           = TB_SHADOW;        // Load on CTR = 0
    EPwm1Regs.TBCTL.bit.PHSDIR          = TB_UP;            // count up after sync
    EPwm1Regs.TBPHS.half.TBPHS          = 0x0000;           // Clear timer phase

    EPwm1Regs.TBCTL.bit.HSPCLKDIV       = TB_DIV1;          // Clock ratio = SYSCLKOUT = 90 MHz, both HSPCLKDIV and CLKDIV = 1
    EPwm1Regs.TBCTL.bit.CLKDIV          = TB_DIV1;          // CLKDIV = 1
    EPwm1Regs.TBCTL.bit.SYNCOSEL        = TB_SYNC_IN;       // Sync out on SYNC_IN
    EPwm1Regs.TBCTL.bit.FREE_SOFT       = 1;                // Stop counting after CTR = 0

    EPwm1Regs.CMPCTL.bit.SHDWAMODE      = CC_SHADOW;
    EPwm1Regs.CMPCTL.bit.SHDWBMODE      = CC_SHADOW;
    EPwm1Regs.CMPCTL.bit.LOADAMODE      = CC_CTR_ZERO;      // load on CTR = 0
    EPwm1Regs.CMPCTL.bit.LOADBMODE      = CC_CTR_ZERO;      // load on CTR = 0

    EPwm1Regs.AQCTLA.bit.CAU            = AQ_SET;           // Action on __
    EPwm1Regs.AQCTLA.bit.CAD            = AQ_CLEAR;         // Action on __
    EPwm1Regs.AQCTLA.bit.ZRO            = AQ_CLEAR;         // Action on __
    EPwm1Regs.AQSFRC.bit.RLDCSF         = 0;                //Load on CTR = 0

    // Contol of ePWM channel B comes from channel А:
    // Channel A sets the clocks, next on DB level there is an inversion with dead zone
    EPwm1Regs.DBCTL.bit.HALFCYCLE       = 0;                // TBCLCK rate
    EPwm1Regs.DBCTL.bit.IN_MODE         = 0;                // PWMxA is source for RED and FED
    EPwm1Regs.DBCTL.bit.OUT_MODE        = DB_FULL_ENABLE;   // enable Dead-band RED & FED (DBM fully enabled)

    //=========================================================================
    // PWM channel B inversion !!
    EPwm1Regs.DBCTL.bit.POLSEL          = DB_ACTV_HIC;      // Active Hi complementary - B = /A

    // DEAD TIME !!
    EPwm1Regs.DBFED                     = 99;                // FED; 1 = 1/90MHz = 11 ns
    EPwm1Regs.DBRED                     = 99;                // RED; 99 = 1 us for MOSFET; for IGBT need 300 ~= 3 us
    //=========================================================================

    //--------------------------------------------------------------------------------
    //Trip zone
    EALLOW;
    Comp2Regs.COMPCTL.bit.COMPDACEN     = 1;            //Comparator + DAC enable
    Comp2Regs.COMPCTL.bit.COMPSOURCE    = 0;            //DAC as a ref source
    Comp2Regs.COMPCTL.bit.QUALSEL       = 0;            //2 clocks sample window, 22 ns
    Comp2Regs.COMPCTL.bit.SYNCSEL       = 0;            //Async
    Comp2Regs.DACCTL.bit.DACSOURCE      = 0;            //DACVAL
    GpioCtrlRegs.AIOMUX1.bit.AIO4       = 3;            //COMP2A
    GpioCtrlRegs.AIODIR.bit.AIO4        = 0;            //Input for COMP2
    //---------------------------------------------
    //Trip value Uref, over which TZ trips
    //V = DACVAL * (VDDA - VSSA) / 1023 = DACVAL * (3.3V - 0V) / 1023
    //DACVAL = 1023 -> 3.3 V -> MAX
    //---------------------------------------------
    Comp2Regs.DACVAL.bit.DACVAL         = 512;          //0.5 Umax = 1.65 V
    //---------------------------------------------
    //Trip zone control based on COMP2OUT
    EPwm1Regs.DCTRIPSEL.bit.DCAHCOMPSEL = 0x9;          //COMP2OUT
    EPwm1Regs.TZDCSEL.bit.DCAEVT1       = 0x2;          //DCBH = high, DCBL  = don’t care
    EPwm1Regs.DCACTL.bit.EVT1SRCSEL     = 0;            //DCAEVT1
    EPwm1Regs.DCACTL.bit.EVT1FRCSYNCSEL = 1;            //Async - immediate trip
    //---------------------------------------------
    EPwm1Regs.TZCTL.bit.DCAEVT1         = TZ_FORCE_LO;  //Force Low at trip
    //---------------------------------------------
    EPwm1Regs.TZSEL.bit.DCAEVT1         = 1;            //enable as a trip source
    EPwm1Regs.TZEINT.bit.DCAEVT1        = 1;            //INT enable
    EPwm1Regs.TZCLR.bit.OST             = 1;            //One-shot flag clear
    EPwm1Regs.TZCLR.all                 = 0xF;          //INT flag clear
    EDIS;

    //--------------------------------------------------------------------------------
    //ADC SOCA
    EPwm1Regs.ETSEL.bit.SOCAEN          = 1;    //enable SOCA
    EPwm1Regs.ETSEL.bit.SOCASEL         = 2;    //SOCA on CTR = PRD
    EPwm1Regs.ETPS.bit.SOCAPRD          = 1;    //SOC on first event


    //====================================================
    //EPWM2
    //--------------------------------------------------------------------------------
    EPwm2Regs.TBPRD                     = 45000000 / p->Pwm_freq;   // Period - 11250 for 4KHz, 4500 for 10 kHz
    EPwm2Regs.TBPHS.half.TBPHS          = 0;                        // Phase
    EPwm2Regs.TBCTR                     = 0;                        // zero to cnt

    EPwm2Regs.CMPA.half.CMPA            = EPwm2Regs.TBPRD + 1;      // Set CTR level
    //--------------------------------------------------------------------------------

    EPwm2Regs.TBCTL.bit.CTRMODE         = TB_COUNT_UPDOWN;  // Symmetrical mode
    EPwm2Regs.TBCTL.bit.PHSEN           = TB_DISABLE;       // Phase control
    EPwm2Regs.TBCTL.bit.PRDLD           = TB_SHADOW;        // Load on CTR = 0
    EPwm2Regs.TBCTL.bit.PHSDIR          = TB_UP;            // count up after sync
    EPwm2Regs.TBPHS.half.TBPHS          = 0x0000;           // Clear timer phase

    EPwm2Regs.TBCTL.bit.HSPCLKDIV       = TB_DIV1;          // Clock ratio = SYSCLKOUT = 90 MHz, both HSPCLKDIV and CLKDIV = 1
    EPwm2Regs.TBCTL.bit.CLKDIV          = TB_DIV1;          // CLKDIV = 1
    EPwm2Regs.TBCTL.bit.SYNCOSEL        = TB_SYNC_IN;       // Sync out on SYNC_IN
    EPwm2Regs.TBCTL.bit.FREE_SOFT       = 1;                // Stop counting after CTR = 0

    EPwm2Regs.CMPCTL.bit.SHDWAMODE      = CC_SHADOW;
    EPwm2Regs.CMPCTL.bit.SHDWBMODE      = CC_SHADOW;
    EPwm2Regs.CMPCTL.bit.LOADAMODE      = CC_CTR_ZERO;      // load on CTR = 0
    EPwm2Regs.CMPCTL.bit.LOADBMODE      = CC_CTR_ZERO;      // load on CTR = 0

    EPwm2Regs.AQCTLA.bit.CAU            = AQ_SET;           // Action on __
    EPwm2Regs.AQCTLA.bit.CAD            = AQ_CLEAR;         // Action on __
    EPwm2Regs.AQCTLA.bit.ZRO            = AQ_CLEAR;         // Action on __
    EPwm2Regs.AQSFRC.bit.RLDCSF         = 0;                // Load on CTR = 0

    // Contol of ePWM channel B comes from channel А:
    // Channel A sets the clocks, next on DB level there is an inversion with dead zone
    EPwm2Regs.DBCTL.bit.HALFCYCLE       = 0;                // TBCLCK rate
    EPwm2Regs.DBCTL.bit.IN_MODE         = 0;                // PWMxA is source for RED and FED
    EPwm2Regs.DBCTL.bit.OUT_MODE        = DB_FULL_ENABLE;   // Enable Dead-band RED & FED (DBM fully enabled)

    //=========================================================================
    // PWM channel B inversion !!
    EPwm2Regs.DBCTL.bit.POLSEL          = DB_ACTV_HIC;      // Active Hi complementary - B = /A
    // DEAD TIME !!
    EPwm2Regs.DBFED                     = 99;               // FED; 1 = 1/90MHz = 11 ns
    EPwm2Regs.DBRED                     = 99;               // RED; 99 = 1 us for MOSFET; for IGBT need 300 ~= 3 us
    //=========================================================================
    //--------------------------------------------------------------------------------
    //Trip zone
    EALLOW;
    Comp3Regs.COMPCTL.bit.COMPDACEN     = 1;            //Comparator + DAC enable
    Comp3Regs.COMPCTL.bit.COMPSOURCE    = 0;            //DAC as a ref source
    Comp3Regs.COMPCTL.bit.QUALSEL       = 0;            //2 clocks sample window, 22 ns
    Comp3Regs.COMPCTL.bit.SYNCSEL       = 0;            //Async
    Comp3Regs.DACCTL.bit.DACSOURCE      = 0;            //DACVAL
    GpioCtrlRegs.AIOMUX1.bit.AIO6       = 3;            //COMP3A
    GpioCtrlRegs.AIODIR.bit.AIO6        = 0;            //Input for COMP3
    //---------------------------------------------
    //Trip value Uref, over which TZ trips
    //V = DACVAL * (VDDA - VSSA) / 1023 = DACVAL * (3.3V - 0V) / 1023
    //DACVAL = 1023 -> 3.3 V -> MAX
    //---------------------------------------------
    Comp3Regs.DACVAL.bit.DACVAL         = 512;          //0.5 Umax = 1.65 V
    //---------------------------------------------
    //Trip zone control based on COMP3OUT
    EPwm2Regs.DCTRIPSEL.bit.DCAHCOMPSEL = 0xA;          //COMP3OUT
    EPwm2Regs.TZDCSEL.bit.DCAEVT1       = 0x2;          //DCBH = high, DCBL  = don’t care
    EPwm2Regs.DCACTL.bit.EVT1SRCSEL     = 0;            //DCAEVT1
    EPwm2Regs.DCACTL.bit.EVT1FRCSYNCSEL = 1;            //Async - immediate trip
    //---------------------------------------------
    EPwm2Regs.TZCTL.bit.DCAEVT1         = TZ_FORCE_LO;  //Force Low at trip
    //---------------------------------------------
    EPwm2Regs.TZSEL.bit.DCAEVT1         = 1;            //enable as a trip source for both channels A and B
    EPwm2Regs.TZEINT.bit.DCAEVT1        = 1;            //INT enable
    EPwm2Regs.TZCLR.bit.OST             = 1;            //One-shot flag clear
    EPwm2Regs.TZCLR.all                 = 0xF;          //INT flag clear
    EDIS;
    //----------------------------------------------
    //ADC SOCA
    EPwm2Regs.ETSEL.bit.SOCAEN          = 1;    //enable SOCA
    EPwm2Regs.ETSEL.bit.SOCASEL         = 2;    //SOCA on CTR = PRD
    EPwm2Regs.ETPS.bit.SOCAPRD          = 1;    //SOC on first event


    //====================================================
    //EPWM3
    //--------------------------------------------------------------------------------
    EPwm3Regs.TBPRD                     = 45000000 / p->Pwm_freq;   // Period - 11250 for 4KHz, 4500 for 10 kHz
    EPwm3Regs.TBPHS.half.TBPHS          = 0;                        // Phase
    EPwm3Regs.TBCTR                     = 0;                        // cnt to zero

    EPwm3Regs.CMPA.half.CMPA            = EPwm3Regs.TBPRD + 1;      // Set CTR level
    //--------------------------------------------------------------------------------

    EPwm3Regs.TBCTL.bit.CTRMODE         = TB_COUNT_UPDOWN;  // Symmetrical mode
    EPwm3Regs.TBCTL.bit.PHSEN           = TB_DISABLE;       // Phase control
    EPwm3Regs.TBCTL.bit.PRDLD           = TB_SHADOW;        // Load on CTR = 0
    EPwm3Regs.TBCTL.bit.PHSDIR          = TB_UP;            // count up after sync
    EPwm3Regs.TBPHS.half.TBPHS          = 0x0000;           // Clear timer phase

    EPwm3Regs.TBCTL.bit.HSPCLKDIV       = TB_DIV1;          // Clock ratio = SYSCLKOUT = 90 MHz, both HSPCLKDIV and CLKDIV = 1
    EPwm3Regs.TBCTL.bit.CLKDIV          = TB_DIV1;          // CLKDIV = 1
    EPwm3Regs.TBCTL.bit.SYNCOSEL        = TB_SYNC_IN;       // Sync out on SYNC_IN
    EPwm3Regs.TBCTL.bit.FREE_SOFT       = 1;                // Остановка шим после CTR = 0

    EPwm3Regs.CMPCTL.bit.SHDWAMODE      = CC_SHADOW;
    EPwm3Regs.CMPCTL.bit.SHDWBMODE      = CC_SHADOW;
    EPwm3Regs.CMPCTL.bit.LOADAMODE      = CC_CTR_ZERO;      // load on CTR = 0
    EPwm3Regs.CMPCTL.bit.LOADBMODE      = CC_CTR_ZERO;      // load on CTR = 0

    EPwm3Regs.AQCTLA.bit.CAU            = AQ_SET;           // Action on __
    EPwm3Regs.AQCTLA.bit.CAD            = AQ_CLEAR;         // Action on __
    EPwm3Regs.AQCTLA.bit.ZRO            = AQ_CLEAR;         // Action on __
    EPwm3Regs.AQSFRC.bit.RLDCSF         = 0;                //Load on CTR = 0

    // Contol of ePWM channel B comes from channel А:
    // Channel A sets the clocks, next on DB level there is an inversion with dead zone
    EPwm3Regs.DBCTL.bit.HALFCYCLE       = 0;                // TBCLCK rate
    EPwm3Regs.DBCTL.bit.IN_MODE         = 0;                // PWMxA is source for RED and FED
    EPwm3Regs.DBCTL.bit.OUT_MODE        = DB_FULL_ENABLE;   // enable Dead-band RED & FED (DBM fully enabled)

    //=========================================================================
    // Channel - B inversion of channel A
    EPwm3Regs.DBCTL.bit.POLSEL          = DB_ACTV_HIC;      // Active Hi complementary - B = /A

    // DEAD TIME !!
    EPwm3Regs.DBFED                     = 99;                // FED; 1 = 1/90MHz = 11 ns
    EPwm3Regs.DBRED                     = 99;                // RED; 99 = 1 us for MOSFET; for IGBT need 300 ~= 3 us
    //=========================================================================

    //Trip Zone
    //Due to on this board COMP1 is occupied by ADC for Ub, we use COMP2OUT OR COMP3OUT logic as a ePWM3 Trip Zone sources
    // Int is not used, cause both COMP2OUT and COMP3OUT have their own interrupts
    EPwm3Regs.DCTRIPSEL.bit.DCAHCOMPSEL = 0x9;          //COMP2OUT
    EPwm3Regs.DCTRIPSEL.bit.DCBHCOMPSEL = 0xA;          //COMP3OUT
    EPwm3Regs.TZDCSEL.bit.DCAEVT1       = 0x2;          //DCBH = high, DCBL  = don’t care
    EPwm3Regs.TZDCSEL.bit.DCBEVT1       = 0x2;          //DCBH = high, DCBL  = don’t care
    EPwm3Regs.DCACTL.bit.EVT1SRCSEL     = 0;            //DCAEVT1
    EPwm3Regs.DCBCTL.bit.EVT1SRCSEL     = 0;            //DCBEVT1
    EPwm3Regs.DCACTL.bit.EVT1FRCSYNCSEL = 1;            //Async - immediate trip
    EPwm3Regs.DCBCTL.bit.EVT1FRCSYNCSEL = 1;            //Async - immediate trip
    //---------------------------------------------
    EPwm3Regs.TZCTL.bit.DCAEVT1         = TZ_FORCE_LO;  //Force Low at trip
    EPwm3Regs.TZCTL.bit.DCBEVT1         = TZ_FORCE_LO;  //Force Low at trip
    //---------------------------------------------
    EPwm3Regs.TZSEL.bit.DCAEVT1         = 1;            //enable as a trip source
    EPwm3Regs.TZSEL.bit.DCBEVT1         = 1;            //enable as a trip source
    //EPwm3Regs.TZEINT.bit.DCAEVT1        = 1;          //INT enable
    EPwm3Regs.TZCLR.bit.OST             = 1;            //One-shot flag clear
    EPwm3Regs.TZCLR.all                 = 0xF;          //INT flag clear
    //--------------------------------------------------------------------------------
    //ADC SOCA
    EPwm3Regs.ETSEL.bit.SOCAEN          = 1;            //enable SOCA
    EPwm3Regs.ETSEL.bit.SOCASEL         = 2;            //SOCA on CTR = PRD
    EPwm3Regs.ETPS.bit.SOCAPRD          = 1;            //SOC on first event


    //====================================================
    // EPWM4
    // EPWM4 - Main PWM for Control System clocking, is synchronous with all 3 main PWMs 1-3
    EPwm4Regs.TBPRD                     = EPwm1Regs.TBPRD;          // Period - 11250 for 4KHz, 45000 for 1 kHz
    EPwm4Regs.TBPHS.half.TBPHS          = 0;                        // Phase
    EPwm4Regs.TBCTR                     = 0;                        // cnt to zero
    EPwm4Regs.CMPA.half.CMPA            = EPwm4Regs.TBPRD / 2;      // Set CTR level
    //--------------------------------------------------------------------------------

    EPwm4Regs.TBCTL.bit.CTRMODE         = TB_COUNT_UPDOWN;  // Symmetrical mode
    EPwm4Regs.TBCTL.bit.PHSEN           = TB_DISABLE;       // Phase control
    EPwm4Regs.TBCTL.bit.PRDLD           = TB_SHADOW;        // Load on CTR = 0
    EPwm4Regs.TBCTL.bit.PHSDIR          = TB_UP;            // count up after sync
    EPwm4Regs.TBPHS.half.TBPHS          = 0x0000;           // Clear timer phase

    EPwm4Regs.TBCTL.bit.HSPCLKDIV       = TB_DIV1;          // Clock ratio = SYSCLKOUT = 90 MHz, both HSPCLKDIV and CLKDIV = 1
    EPwm4Regs.TBCTL.bit.CLKDIV          = TB_DIV1;          // CLKDIV = 1
    EPwm4Regs.TBCTL.bit.SYNCOSEL        = TB_SYNC_IN;       // Sync out on SYNC_IN

    EPwm4Regs.CMPCTL.bit.SHDWAMODE      = CC_SHADOW;
    EPwm4Regs.CMPCTL.bit.SHDWBMODE      = CC_SHADOW;
    EPwm4Regs.CMPCTL.bit.LOADAMODE      = CC_CTR_ZERO;      // load on CTR = 0
    EPwm4Regs.CMPCTL.bit.LOADBMODE      = CC_CTR_ZERO;      // load on CTR = 0

    EPwm4Regs.ETSEL.bit.INTEN           = 1;                // Enable EPwm4 INT
    EPwm4Regs.ETSEL.bit.INTSEL          = 2;                // INT on CTR = PERIOD
    EPwm4Regs.ETPS.bit.INTCNT           = 1;                // INT on 1st event
    EPwm4Regs.ETPS.bit.INTPRD           = 1;                // INT on 1st event

    EPwm4Regs.ETSEL.bit.SOCAEN          = 1;                //enable SOCA for Udc measurement
    EPwm4Regs.ETSEL.bit.SOCASEL         = 2;                //SOCA on CTR = PRD
    EPwm4Regs.ETPS.bit.SOCAPRD          = 1;                //SOC on first event


    //====================================================
    // EPWM5
    // EPWM5 - PWM for clocking test ADC
   EPwm5Regs.TBPRD                     = EPwm1Regs.TBPRD * 20;            // Period - 11250 for 4KHz, 45000 for 1 kHz
   EPwm5Regs.TBPHS.half.TBPHS          = 0;                // Phase
   EPwm5Regs.TBCTR                     = 0;                // cnt to zero
   EPwm5Regs.CMPA.half.CMPA            = EPwm5Regs.TBPRD-1;

   EPwm5Regs.TBCTL.bit.CTRMODE         = TB_COUNT_UPDOWN;  // Symmetrical mode
   EPwm5Regs.TBCTL.bit.PHSEN           = TB_DISABLE;       // Phase control
   EPwm5Regs.TBCTL.bit.PRDLD           = TB_SHADOW;        // Load on CTR = 0
   EPwm5Regs.TBCTL.bit.PHSDIR          = TB_UP;            // count up after sync
   EPwm5Regs.TBPHS.half.TBPHS          = 0x0000;           // Clear timer phase

   EPwm5Regs.TBCTL.bit.HSPCLKDIV       = TB_DIV1;          // Clock ratio = SYSCLKOUT = 90 MHz, both HSPCLKDIV and CLKDIV = 1
   EPwm5Regs.TBCTL.bit.CLKDIV          = TB_DIV1;          // CLKDIV = 1
   EPwm5Regs.TBCTL.bit.SYNCOSEL        = TB_SYNC_DISABLE;  // Sync out disabled - last PWM in a chain of syncing
   EPwm5Regs.TBCTL.bit.FREE_SOFT       = 0;                // Stop just after emul stop

   EPwm5Regs.ETSEL.bit.SOCAEN          = 1;                 //enable SOCA
   EPwm5Regs.ETSEL.bit.SOCASEL         = 2;                 //SOCA on CTR = PRD
   EPwm5Regs.ETPS.bit.SOCAPRD          = 1;                 //SOC on first event

   //====================================================
   //Simultanious switch ON of all PWMs, all start to count from zero
    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1; //включение pwm clock
    EDIS;
}

void Pwm_Fast_calc(TPwm *p)
{
    // Set of PWM comparator levels, based on normalized base voltage : 0 = 0 V, 1 = 24 V
    // Check of normalization of U_ref
    if (p->Ua_ref > 1.0) p->Ua_ref    = 1.0;
    if (p->Ub_ref > 1.0) p->Ub_ref    = 1.0;
    if (p->Uc_ref > 1.0) p->Uc_ref    = 1.0;

    EPwm1Regs.CMPA.half.CMPA    = EPwm1Regs.TBPRD * (1 - p->Ua_ref) + 1; //+1 for full zero to make 1 tick of CMPA = TBPRD not to trigger transistor opening
    EPwm2Regs.CMPA.half.CMPA    = EPwm2Regs.TBPRD * (1 - p->Ub_ref) + 1;
    EPwm3Regs.CMPA.half.CMPA    = EPwm3Regs.TBPRD * (1 - p->Uc_ref) + 1;
}

void Pwm_Khz_calc(TPwm *p)
{

}

void Pwm_Slow_calc(TPwm *p)
{

}

void Pwm_Off(TPwm *p)
{
    EALLOW;
    EPwm1Regs.AQCSFRC.bit.CSFA = 1;     //force continious low on pin
    EPwm1Regs.AQCSFRC.bit.CSFB = 1;     //
    EPwm2Regs.AQCSFRC.bit.CSFA = 1;     //force continious low on pin
    EPwm2Regs.AQCSFRC.bit.CSFB = 1;     //
    EPwm3Regs.AQCSFRC.bit.CSFA = 1;     //force continious low on pin
    EPwm3Regs.AQCSFRC.bit.CSFB = 1;     //
    GpioDataRegs.GPBDAT.bit.GPIO50 = 0; // EN_GATE for Booster Board
    EN_GATE = 0;
    EDIS;
}

void Pwm_On(TPwm *p)
{
    EALLOW;
    EPwm1Regs.AQCSFRC.bit.CSFA = 0;     //unforce continious low on pin
    EPwm1Regs.AQCSFRC.bit.CSFB = 0;     //
    EPwm2Regs.AQCSFRC.bit.CSFA = 0;     //unforce continious low on pin
    EPwm2Regs.AQCSFRC.bit.CSFB = 0;     //
    EPwm3Regs.AQCSFRC.bit.CSFA = 0;     //unforce continious low on pin
    EPwm3Regs.AQCSFRC.bit.CSFB = 0;     //
    GpioDataRegs.GPBDAT.bit.GPIO50 = 1; // EN_GATE for Booster Board
    EN_GATE = 1;
    EDIS;
}


