//
//Nominal values of a drive
//

#include "V_Include/V_Drv_Param.h"
#include "V_Include/main.h"

void Drv_Param_Init(TDrv_Param *p)
{
    drv_param.Umax_hmi = 12.0;  //Max phase voltage, V
    drv_param.Imax_hmi = 3.0;   //Max current, A; is determined by max power source current
    drv_param.Speed_max_hmi = 1000; //Max speed, rpm; limitation for safety reasons
}

void Drv_Param_Fast_calc(TDrv_Param *p)
{

}

void Drv_Param_Khz_calc(TDrv_Param *p)
{

}

void Drv_Param_Slow_calc(TDrv_Param *p)
{
    //Update of Nominal values of a drive
    drv_param.Umax = _IQ(drv_param.Umax_hmi/drv_param.Umot_nom);
    drv_param.Imax = _IQ(drv_param.Imax_hmi/drv_param.Imot_nom);
    drv_param.Speed_max = _IQ(drv_param.Speed_max_hmi/drv_param.Speedmot_nom_hmi);
}
