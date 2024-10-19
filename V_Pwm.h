//
// V_Pwm.h - PWM Header file
//

#ifndef V_INCLUDE_V_PWM_H
#define V_INCLUDE_V_PWM_H

struct SPwm
  {
    int Pwm_freq;                       //PWM freq, Hz
    float Ua_ref, Ub_ref, Uc_ref;

    void (*init)(struct SPwm*);         // Pointer to the init funcion
    void (*slow_calc)(struct SPwm*);    // Pointer to the slow calc funtion
    void (*fast_calc)(struct SPwm*);    // Pointer to the fast calc funtion
    void (*khz_calc)(struct SPwm*);     // Pointer to the khz calc funtion
    void (*off)(struct SPwm*);
    void (*on)(struct SPwm*);
};

typedef struct SPwm TPwm;

#define PWM_DEFAULTS {\
    10000,\
    0, 0, 0,\
    Pwm_Init,\
    Pwm_Slow_calc,\
    Pwm_Fast_calc,\
    Pwm_Khz_calc,\
    Pwm_Off,\
    Pwm_On\
  }

  void Pwm_Init(TPwm*);

  void Pwm_Slow_calc(TPwm*);

  void Pwm_Fast_calc(TPwm*);

  void Pwm_Khz_calc(TPwm*);

  void Pwm_Off(TPwm*);

  void Pwm_On(TPwm*);

#endif /* V_INCLUDE_V_PWM_H_ */
