//
// V_Adc.h - Header file of ADCs
//
#include "main.h"

#ifndef V_INCLUDE_V_ADC_H
#define V_INCLUDE_V_ADC_H

struct SAdc
  {
    float Umeas_dc_gain;
    int16 Umeas_dc_offset;
    float Umeas_dc;         // Measured value, normalized, used in further calc
    int32 Udctemp;          // Temporary variable for scaling 0-4095 -> -32768 - +32752
    float UdcGainNom;       // Normalization gain

    float Umeas_a_gain;
    int16 Umeas_a_offset;
    float Umeas_a;          // Measured value, normalized, used in further calc
    int32 Uatemp;           // Temporary variable for scaling 0-4095 -> -32768 - +32752
    float UaGainNom;        // Normalization gain

    float Umeas_b_gain;
    int16 Umeas_b_offset;
    float Umeas_b;          // Measured value, normalized, used in further calc
    int32 Ubtemp;           // Temporary variable for scaling 0-4095 -> -32768 - +32752
    float UbGainNom;        // Normalization gain

    float Umeas_c_gain;
    int16 Umeas_c_offset;
    float Umeas_c;          // Measured value, normalized, used in further calc
    int32 Uctemp;           // Temporary variable for scaling 0-4095 -> -32768 - +32752
    float UcGainNom;        // Normalization gain

    float Imeas_a_gain;
    int16 Imeas_a_offset;
    float Imeas_a;          // Measured value, normalized, used in further calc
    int32 Itemp_a;          // Temporary variable for scaling 0-4095 -> -32768 - +32752
    float IaGainNom;        // Normalization gain

    float Imeas_b_gain;
    int16 Imeas_b_offset;
    float Imeas_b;          // Measured value, normalized, used in further calc
    int32 Itemp_b;          // Temporary variable for scaling 0-4095 -> -32768 - +32752
    float IbGainNom;        // Normalization gain

    float Imeas_c_gain;
    int16 Imeas_c_offset;
    float Imeas_c;          // Measured value, normalized, used in further calc
    int32 Itemp_c;          // Temporary variable for scaling 0-4095 -> -32768 - +32752
    float IcGainNom;        // Normalization gain

    //float T_meas_gain;
    //int16 Tmeas_offset;
    //float T_meas;                   //!< Averaged measured value
    //int32 T_temp;                   //!<Temporary variable
    //float TGainNom;                 // Normalization gain

    void (*init)(struct SAdc*);          // Pointer to the init funcion
    void (*slow_calc)(struct SAdc*);     // Pointer to the slow calc funtion
    void (*fast_calc)(struct SAdc*);     // Pointer to the fast calc funtion
    void (*khz_calc)(struct SAdc*);      // Pointer to the khz calc funtion
};

typedef struct SAdc TAdc;

//3.3 V ADC in -> 26.314 V
// +-1.65 V ADC in -> +-16.5 A
#define ADC_DEFAULTS {\
    26.314,0,0,0,0,\
    26.314,0,0,0,0,\
    26.314,0,0,0,0,\
    26.314,0,0,0,0,\
    16.0,0,0,0,0,\
    16.0,0,0,0,0,\
    16.0,0,0,0,0,\
    Adc_Init,\
    Adc_Slow_calc,\
    Adc_Fast_calc,\
    Adc_Khz_calc};

void Adc_Init(TAdc*);

void Adc_Slow_calc(TAdc*);

void Adc_Fast_calc(TAdc*);

void Adc_Khz_calc(TAdc*);

#endif /* V_INCLUDE_V_ADC_H_ */
