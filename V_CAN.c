//
//Nominal values of a drive
//

#include "V_Include/V_CAN.h"
#include "V_Include/main.h"

void external_CAN_Init()
{
    /* Create a shadow register structure for the CAN control registers.
    This is needed, since, only 32-bit access is allowed to these registers. 16-bit access
    to these registers could potentially corrupt the register contents. This is
    especially true while writing to a bit (or group of bits) among bits 16 - 31 */
    /* Initialize the CAN module */


}

void CAN_Init(TCAN *p)
{
    struct ECAN_REGS    ECanaShadow;   // Create a shadow register structure for the CAN control registers.
    struct LAM_REGS     ECanaLAMShadow;
                                    // This is needed, since only 32-bit access is allowed to these registers.
    EALLOW;
 //---------------------------------------------------------
    // GPIO
    ECanaShadow.CANTIOC.bit.TXFUNC  = 1;     //Enable Tx ECAN pin
    ECanaShadow.CANRIOC.bit.RXFUNC  = 1;     //Enable Rx ECAN pin

//---------------------------------------------------------
    // Initialization
    ECanaShadow.CANMC.bit.CCR       = 1;
    while (ECanaRegs.CANES.bit.CCE != 1){}  //wait...

    //+++ Baud rate + timing control for resynchronization
    ECanaShadow.CANBTC.bit.BRPREG   = 7;    // TQ = 1/SYSCLOCKOUT/2 * (1+BPRreg) = 2*(1+7)/SYSCLKOUT = 16 x SYSCLKOUT = 176 ns
    ECanaShadow.CANBTC.bit.TSEG1REG = 3;    // Bit length PROP + PHASE1 = (3+1)xTQ = 704 ns
    ECanaShadow.CANBTC.bit.TSEG2REG = 3;    // Bit length PHASE2 = (3+1)xTQ = 704 ns
    //+++ Total baud rate is 1+4+4 Tq = 1584 ns/bit ~ 0.6 Mbit/s
    ECanaShadow.CANBTC.bit.SJWREG   = 1;    // Up to 1+1=2 bits lengthen the bit duration for resynch
    ECanaShadow.CANBTC.bit.SAM      = 0;    // 0 - 1 sample to check the bus

    //Masks
    ECanaLAMShadow.LAM0.all         = 0;
    ECanaLAMShadow.LAM1.bit.LAMI    = 1;    //Std and extended frames
    ECanaLAMShadow.LAM1.bit.LAM_L   = 0x0000;   //Low part of mask
    ECanaLAMShadow.LAM1.bit.LAM_H   = 0x0000;   //Low part of mask

    //Initialization end
    ECanaShadow.CANMC.bit.CCR       = 0;
    while (ECanaRegs.CANES.bit.CCE != 0){}  //wait...
    //Initialization complete, init mode OFF

//---------------------------------------------------------
    //CAN module Control
    ECanaShadow.CANMC.bit.PDR       = 0;    // Power down mode is off
    ECanaShadow.CANMC.bit.DBO       = 0;    // MSByte first
    ECanaShadow.CANMC.bit.WUBA      = 1;    // Wake up on any bus activity
    ECanaShadow.CANMC.bit.CDR       = 0;    // Normal access to data reg
    ECanaShadow.CANMC.bit.ABO       = 1;    // Automatic go from Bus-off state
    ECanaShadow.CANMC.bit.SCB       = 0;    // CAN is SCC standard mode (eCAN is off) - only 16 mailboxes
    ECanaShadow.CANMC.bit.SUSP      = 0;    // Emulation: transmit last frame and stop
    ECanaShadow.CANMC.bit.SRES      = 0;    // Reset
    ECanaShadow.CANMC.bit.MBNR      = 0;    // ECan only

    //+++ Test Mode +++
    ECanaShadow.CANMC.bit.STM       = 0;    // Self -test mode
    //ECanaShadow.CANMC.bit.CCR     = 1;    // Write request to CANBTC

//---------------------------------------------------------
    // Interrupts
    ECanaShadow.CANMIM.bit.MIM0     = 1;    //Mailbox 0 INT enable
    ECanaShadow.CANMIM.bit.MIM1     = 1;    //Mailbox 1 INT enable

    //ECAN0INT - Tx, ECAN1INT - Rx
    ECanaShadow.CANMIL.bit.MIL0     = 0;    //Tx
    ECanaShadow.CANMIL.bit.MIL1     = 1;    //Rx

    //Global CAN INT enable
    ECanaShadow.CANGIM.bit.I0EN     = 1;
    ECanaShadow.CANGIM.bit.I1EN     = 1;

//---------------------------------------------------------
    //Mailbox enable
    ECanaShadow.CANME.bit.ME0       = 1;
    ECanaShadow.CANME.bit.ME1       = 1;

    //Mailbox directions
    ECanaShadow.CANMD.all = ECanaRegs.CANMD.all; //copy reg to shadow
    ECanaShadow.CANMD.bit.MD0       = 0;    //Tx
    ECanaShadow.CANMD.bit.MD1       = 1;    //Rx
    ECanaRegs.CANMD.all = ECanaShadow.CANMD.all;

    ECanaShadow.CANOPC.all = 0x0;           //Overwrite protection is off

//---------------------------------------------------------
    //Mailbox Auto-Answer Mode
    //AAM = 0;

    // Request frame
    //RTR = 0;

    //Transmission aborted
    //CANAA.AA0 == 1 -> Interrupt, aborted

    EDIS;
}

void CAN_Fast_Calc(TCAN *p)
{

}

void CAN_Khz_Calc(TCAN *p)
{

}

void CAN_Slow_Calc(TCAN *p)
{
    //Main CAN handler here

    struct ECAN_REGS    ECanaShadow2;

    //+++ Tx +++
    // Start transmission from Mailbox n
    //Configure mailbox frame ID and CTL sections
    ECanaShadow2.CANTRR.bit.TRR0    = 1;
    ECanaShadow2.CANME.bit.ME0      = 0;

    ECanaMboxes.MBOX0.MSGID.bit.AME = 0;
    ECanaMboxes.MBOX0.MSGID.bit.AAM = 0;    //No remote request answer
    ECanaMboxes.MBOX0.MSGID.bit.IDE = 1;    //Extended frame
    ECanaMboxes.MBOX0.MSGID.bit.EXTMSGID_H  = 0x0;
    ECanaMboxes.MBOX0.MSGID.bit.EXTMSGID_L  = 0x3;
    ECanaMboxes.MBOX0.MSGCTRL.bit.DLC   = 1;    //Data length in bytes
    ECanaMboxes.MBOX0.MSGCTRL.bit.RTR   = 0;

    ECanaShadow2.CANME.bit.ME0      = 1;

    //Configure mailbox frame data


    ECanaShadow2.CANTRS.bit.TRS0    = 1;
    if(ECanaShadow2.CANTA.bit.TA0 == 1); //transmission was successful, INT is generated

    //+++ Rx +++
    if(ECanaShadow2.CANRMP.bit.RMP0 == 1) //pending rx message, INT is generated
    {
        //There is pending recieved message in mailbox 0

        //Priority: A new incoming message overwrites the stored one if the
        //OPC[n](OPC.31-0) bit is cleared, otherwise the next mailboxes are checked for a matching ID

        // Check CANRML for Overwritten messages
        //ECanaRegs.CANGAM.bit.AMI = 0; //EDI-based 11/28 ID - no mask - NEEDS CCR INIT TO MODIFY!!!
    }
}

void CANINT0_Handler (void)
{
    //Clear CAN INT Tx ackn flag
    ECanaRegs.CANTA.bit.TA0 = 1;
}

void CANINT1_Handler (void)
{
    //Clear CAN INT Rx ackn flag
    //ECanaRegs.CANR
}
