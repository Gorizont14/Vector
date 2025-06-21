//Clarke transfomr - phase transform abc to alpha-beta


#include "V_Include/V_Clarke.h"
#include "V_Include/V_IQmath.h"        /* Include header for IQmath library */

#pragma CODE_SECTION(Clarke_Calc, "ramfuncs");

// Phase transform function
void Clarke_Calc(TClarke *v) {
	v->ds = v->as;
	v->qs = _IQmpy((v->as + _IQmpy(_IQ(2),v->bs)), _IQ(0.57735026918963));
}

/*@}*/

