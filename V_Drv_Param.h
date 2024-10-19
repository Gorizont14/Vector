//
// Nominal values of a drive and motor
//

#include "main.h"

#ifndef V_INCLUDE_V_DRV_PARAM_H
#define V_INCLUDE_V_DRV_PARAM_H

struct SDrv_Param
  {
    float I_nom;            // Drive Nominal Current, A
    float Udc_nom;          // Drive Nominal Udc, V
    float Imot_nom;         // Motor Nominal current, A
    float Umot_nom;         // Motor Nominal Voltage, Line, V
    float Pmot_nom;         // Motor Nominal Power, kW

    void (*init)(struct SDrv_Param*);          // Pointer to the init funcion
    void (*slow_calc)(struct SDrv_Param*);     // Pointer to the slow calc funtion
    void (*fast_calc)(struct SDrv_Param*);     // Pointer to the fast calc funtion
    void (*khz_calc)(struct SDrv_Param*);      // Pointer to the khz calc funtion
};

typedef struct SDrv_Param TDrv_Param;

#define DRV_PARAM_DEFAULTS {\
    10.0, \
    24.0, \
    10.0, \
    40.0, \
    400.0, \
    Drv_Param_Init,\
    Drv_Param_Slow_calc,\
    Drv_Param_Fast_calc,\
    Drv_Param_Khz_calc\
  }

  void Drv_Param_Init(TDrv_Param*);

  void Drv_Param_Slow_calc(TDrv_Param*);

  void Drv_Param_Fast_calc(TDrv_Param*);

  void Drv_Param_Khz_calc(TDrv_Param*);

#endif /* V_INCLUDE_V_DRV_PARAM_H_ */
