//
// SM_Sys.h - Header file for control system
//

#include "main.h"

#ifndef V_INCLUDE_SM_SYS_H
#define V_INCLUDE_SM_SYS_H

//! System is in INIT state
#define SYS_INIT      0
//! System is initialized and READY
#define SYS_READY     1

// class TSM_Sys - Common module for calculation of other program modules

struct SSM_Sys
{
    int state;
    int state_prev;
    int E;              // first entry flag
    void (*init)(struct SSM_Sys*);          // Pointer to the init funcion
    void (*slow_calc)(struct SSM_Sys*);     // Pointer to the slow calc funtion
    void (*fast_calc)(struct SSM_Sys*);     // Pointer to the fast calc funtion
    void (*khz_calc)(struct SSM_Sys*);      // Pointer to the khz calc funtion
};

typedef struct SSM_Sys TSM_Sys;

// Initializer
#define SM_SYS_DEFAULTS {0,0,0,\
    SM_Sys_Init,\
    SM_Sys_Slow_Calc,\
    SM_Sys_Fast_Calc,\
    SM_Sys_Khz_Calc,\
  }


void SM_Sys_Init(TSM_Sys*);

void SM_Sys_Slow_Calc(TSM_Sys*);

void SM_Sys_Fast_Calc(TSM_Sys*);

void SM_Sys_Khz_Calc(TSM_Sys*);


#endif /* V_INCLUDE_SM_SYS_H */
