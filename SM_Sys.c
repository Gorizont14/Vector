//
//Main count of Control System - Init, Fast Calc, Slow Calc
//

#include "V_Include/SM_Sys.h"
#include "V_Include/main.h"

void SM_Sys_SPI_Init(void)
{
    EALLOW;
    //SPI Switch OFF
    SpiaRegs.SPICCR.bit.SPISWRESET  = 0;    //Reset SPI and keep it OFF until end of init

    SpiaRegs.SPIBRR                 = 0;    //SPICLK = LSPCLK / 4 - 176 ns/tick, tH = 88 ns - x2 of min high time
    SpiaRegs.SPICCR.bit.CLKPOLARITY = 0;    //SPICLK Active on 1
    SpiaRegs.SPICTL.bit.CLK_PHASE   = 0;
    //CLOCK PHASE = 0:
    //SPICLK Up
    //- SDO ready +20ns after SPICLK Up till SPICLK Down +40ns
    //- MDO ready +10ns after SPICLK Up till SPICLK Down +78ns
    //SPICLK Down
    //- MI latches SDO -26ns to SPICLK Down
    //- SI latches MDO -20ns +30ns to SPICLK Down
    SpiaRegs.SPICCR.bit.SPICHAR     = 0xF;  //Character length - 16 bit
    SpiaRegs.SPICTL.bit.MASTER_SLAVE= 1;    //SPI configured as a Master

    //FIFO
    SpiaRegs.SPIFFCT.all            = 0;    // All FIFO delays to 0
    SpiaRegs.SPIFFTX.bit.SPIFFENA   = 0;    // Disable Tx FIFO enhancements
    SpiaRegs.SPIFFTX.bit.TXFFIENA   = 0;    // Disable Tx FIFO interrupts
    SpiaRegs.SPIFFRX.bit.RXFFIENA   = 0;    // Disable Rx FIFO interrupts

    //INT
    SpiaRegs.SPICTL.bit.SPIINTENA   = 1;    //INT enable - to manage SPI words content + timings
    //SpiaRegs.SPICTL.bit.OVERRUNINTENA = 1;  //Overrun INT Enabled

    //Priority control
    SpiaRegs.SPIPRI.bit.SOFT        = 0;    //1 - Emul stop -> end the transmission of current character and then stop
    SpiaRegs.SPIPRI.bit.FREE        = 0;    //no free run
    SpiaRegs.SPIPRI.bit.TRIWIRE     = 0;    //normal 4 wire mode
    SpiaRegs.SPIPRI.bit.STEINV      = 0;    //SPISTE is active low (normal)

    //GPIO config
    GpioCtrlRegs.GPAPUD.bit.GPIO19  = 1;    // SPISTEA Pull-up off
    GpioCtrlRegs.GPAPUD.bit.GPIO18  = 1;    // SPICLKA Pull-up off
    GpioCtrlRegs.GPAPUD.bit.GPIO17  = 1;    // SPISOMIA Pull-up off
    GpioCtrlRegs.GPAPUD.bit.GPIO16  = 1;    // SPISIMOA Pull-up off

    GpioCtrlRegs.GPAMUX2.bit.GPIO19 = 1;    // /SPISTEA
    GpioCtrlRegs.GPAMUX2.bit.GPIO18 = 1;    // SPICLKA
    GpioCtrlRegs.GPAMUX2.bit.GPIO17 = 1;    // SPISOMIA
    GpioCtrlRegs.GPAMUX2.bit.GPIO16 = 1;    // SPISIMOA

    //Qualification off - async mode for peripheral control
    GpioCtrlRegs.GPAQSEL2.bit.GPIO19 = 3; // Asynch input GPIO19 SPISTEA
    GpioCtrlRegs.GPAQSEL2.bit.GPIO18 = 3; // Asynch input GPIO18 SPICLKA
    GpioCtrlRegs.GPAQSEL2.bit.GPIO17 = 3; // Asynch input GPIO17 SPISOMIA
    GpioCtrlRegs.GPAQSEL2.bit.GPIO16 = 3; // Asynch input GPIO16 SPISIMOA

    //SPI Switch ON
    SpiaRegs.SPICCR.bit.SPISWRESET  = 1;    //Switch On SPI

    //TEST! Loop back
    //SpiaRegs.SPICCR.bit.SPILBK      = 1;    //Loop back ON
    EDIS;
}

void SpiServ(void)
{
    //Put data to transmit
    SpiOutData = 0b1000100000000000; //0b1000100000000000 - read STS reg 2 with ID

    //Start the transmission
    //Disable interrupts to prevent breakdown of the SPI frame transmission
    //DINT;
    SpiaRegs.SPICTL.bit.TALK = 1;               //Start ON transmission session
    SpiaRegs.SPITXBUF = SpiOutData;             //must be left-justified due to MSB out first

    while(SpiaRegs.SPISTS.bit.INT_FLAG != 1){}  //wait while data fully rx-ed
    //SpiInData = SpiaRegs.SPIRXBUF;

    //stop transmission
    //SpiaRegs.SPICTL.bit.TALK = 0;
    //Enable interrupts
    //EINT;
    test_buttons &= 0xFFFE;
}

__interrupt void SpiaINTHandler (void)
{
    EALLOW;
    SpiInData = SpiaRegs.SPIRXBUF;          //reading data from RX BUF automatically clears SPI INT flag
    //stop transmission
    SpiaRegs.SPICTL.bit.TALK = 0;
    EDIS;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP6; //reset SPI INT Group flag
}

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
     PieVectTable.ADCINT1       = &ADCINT1_Handler;     //ADC INT for tests
     PieVectTable.EPWM4_INT     = &TI10_IRQ_Handler;    //10 kHz Fast Calc IRQ
     PieVectTable.TINT1         = &TI1_IRQ_Handler;     //1 kHz Slow Calc IRQ
     PieVectTable.EPWM1_TZINT   = &EPWM1_TZINT_Handler; //TZ interrupt EPWM1, group 2
     PieVectTable.EPWM2_TZINT   = &EPWM2_TZINT_Handler; //TZ interrupt EPWM2, group 2
     PieVectTable.SPIRXINTA     = &SpiaINTHandler;      //SPI Interrupt
     EDIS;

     //PieCtrlRegs.PIEIER1.bit.INTx1 = 1;   //Enable ADCINT1 - INT of test ADC
     PieCtrlRegs.PIEIER2.bit.INTx1 = 1;     //Enable EPWM1_TZINT
     PieCtrlRegs.PIEIER2.bit.INTx2 = 1;     //Enable EPWM2_TZINT
     PieCtrlRegs.PIEIER3.bit.INTx4 = 1;     //Enable EPWM4 INT - Main interrupt of Control System calc + common ADC (Udc etc.)
     PieCtrlRegs.PIEIER6.bit.INTx1 = 1;     //Enable SPI INT SPIRXA

     //IER |= M_INT1; //Enable Group 1 Interrupts - ADCINT1
     IER |= M_INT2; //Enable Group 2 Interrupts - EPWM TZ
     IER |= M_INT3; //Enable Group 3 Interrupts - EPWM4
     IER |= M_INT6; //Enable Group 6 Interrupts - SPI
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

     //SPI init - diag from DRV driver
     SM_Sys_SPI_Init();

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
