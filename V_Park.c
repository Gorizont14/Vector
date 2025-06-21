//Park transform - alpha-beta to dq


#include "V_Include/V_Park.h"
#include "V_Include/V_IQmath.h"

#pragma CODE_SECTION(Park_Calc, "ramfuncs");

// Phase transform function
// Input - angle ang, vector (ds,qs)
// Output - rotated vector (de,qe)
void Park_Calc(TPark *v) {

    _iq cos_ang, sin_ang;

    // Using look-up IQ sine table
    sin_ang = _IQsinPU(v->ang);
    cos_ang = _IQcosPU(v->ang);
    //sin_ang = 0;
    //cos_ang = _IQ(1);

    v->de = _IQmpy(v->ds,cos_ang) + _IQmpy(v->qs, sin_ang);
    v->qe = _IQmpy(v->qs,cos_ang) - _IQmpy(v->ds, sin_ang);

}
