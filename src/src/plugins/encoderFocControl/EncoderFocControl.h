// Sample plugin
#pragma once
#include "../../Common.h"

#ifndef ENCODER_FOC_CONTROL
#define ENCODER_FOC_CONTROL OFF
#endif

#if ENCODER_FOC_CONTROL == ON

#ifndef ENCODER_FOC_CONTROL_DIR
#define ENCODER_FOC_CONTROL_DIR 1
#endif

#ifndef ENCODER_FOC_CONTROL_ENCODER_TYPE
#define ENCODER_FOC_CONTROL_ENCODER_TYPE OFF
#endif

#ifndef ENCODER_FOC_CONTROL_MULT
#define ENCODER_FOC_CONTROL_MULT 1
#endif

#ifndef ENCODER_FOC_CONTROL_POLLING_RATE_MS
#define ENCODER_FOC_CONTROL_POLLING_RATE_MS 150
#endif

#if ENCODER_FOC_CONTROL_ENCODER_TYPE == AB || ENCODER_FOC_CONTROL_ENCODER_TYPE == AB_ESP32
  #ifndef ENCODER_FOC_CONTROL_ENCODER_PINA
    #error "Pin A for encoder focus control not defined"
  #endif
  #ifndef ENCODER_FOC_CONTROL_ENCODER_PINB
    #error "Pin B for encoder focus control not defined"
  #endif
#endif

#if ENCODER_FOC_CONTROL_ENCODER_TYPE == AB_STM32
  #ifndef ENCODER_FOC_CONTROL_ENCODER_TIM
  #error "Timer for STM32 encoder focus control not defined"
  #endif
#endif

#ifndef ENCODER_FOC_CONTROL_ENCODER_AXIS
  #error "Define which axis number to take over in ENCODER_FOC_CONTROL_ENCODER_AXIS"
#endif

class EncoderFocControl {
public:
    // the initialization method must be present and named: void init();
    void init();
    void loop();

private:
    bool enFault = false;

    long prevPos;
    long posDiff;

    bool should_slew;

    char fp_cmd[40];
};

extern EncoderFocControl encoderFocControl;

#endif