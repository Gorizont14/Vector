//All main current parameters of the drive in one place for convenience

#include "V_Include/V_Protection.h"
#include "V_Include/V_IQmath.h"
#include "V_Include/main.h"
#include "stdlib.h"

#pragma CODE_SECTION(Prot_Fast_Calc, "ramfuncs");

void Prot_Init(TProt *p) {
    p->drv_flt = 1;
    p->drv_octw = 1;
}

void Prot_Fast_Calc(TProt *p){
    p->drv_flt  = GpioDataRegs.GPADAT.bit.GPIO28;
    p->drv_octw = GpioDataRegs.GPADAT.bit.GPIO29;
}

void Prot_Slow_Calc(TProt *p){

}
