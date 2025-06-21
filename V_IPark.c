//Iverse park transform dq -> alpha-beta

#include "V_Include/V_IPark.h"
#include "V_Include/V_IQmath.h"         /* Include header for IQmath library */

#pragma CODE_SECTION(Ipark_Calc, "ramfuncs");

void Ipark_Calc(TIPark *v) {

	_iq cos_ang, sin_ang;

	sin_ang = _IQsinPU(v->ang);
	cos_ang = _IQcosPU(v->ang);

	v->ds = _IQmpy(v->de,cos_ang) - _IQmpy(v->qe, sin_ang);
	v->qs = _IQmpy(v->qe,cos_ang) + _IQmpy(v->de, sin_ang);
}



