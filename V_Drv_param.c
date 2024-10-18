//
//Nominal values of a drive
//

#include "V_Include/V_Drv_Param.h"
#include "V_Include/main.h"

void Drv_Param_Init(TDrv_Param *p)
{

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
    drv_param.I_nom;       // = Nom_data_Buf[0]
    drv_param.Udc_nom;     // = Nom_data_Buf[1]
}
