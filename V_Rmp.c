// ramp generator control for speed ref ramping


#include "V_Include/V_Rmp.h"
#include "V_Include/V_IQmath.h"
#include "stdlib.h"

//ramp calc
//changes out with predefined pace T, until out equals input=

void Rmp_Ctrl_Calc(TRmp *p) {

	if ((labs(p->out - p->input) <= p->step) || (p->T==0))	//entered to reference with accuracy up to step OR ramp is Off
	{
		p->out = p->input;
	}
	else if (p->out < p->input)
	{
		p->out += p->step;
	}
	else
	{
		p->out -= p->step;
	}

}

// Aux function - recals T into step accounting PWM freq (typically 10 kHz)

void Rmp_Ctrl_Slow_Calc(TRmp *p) {
	p->step = _IQdiv(p->Ts, p->T);      //ramp integrator step
}
