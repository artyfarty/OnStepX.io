// A/B Quadrature encoders (fast STM32 hardware decode)
#pragma once

#include "../Encoder.h"

#if AXIS1_ENCODER == AB_STM32 || AXIS2_ENCODER == AB_STM32 || AXIS3_ENCODER == AB_STM32 || \
    AXIS4_ENCODER == AB_STM32 || AXIS5_ENCODER == AB_STM32 || AXIS6_ENCODER == AB_STM32 || \
    AXIS7_ENCODER == AB_STM32 || AXIS8_ENCODER == AB_STM32 || AXIS9_ENCODER == AB_STM32 || \
    (ENCODER_FOC_CONTROL == ON && ENCODER_FOC_CONTROL_ENCODER_TYPE == AB_STM32)

#include <STM32Encoder.h> // https://github.com/madhephaestus/STM32Encoder/tree/master


// for example:
// QuadratureSTM32 encoder1(AXIS1_ENCODER_A_PIN, AXIS1_ENCODER_B_PIN, 1);

class QuadratureSTM32 : public Encoder {
  public:
    QuadratureSTM32(TIM_TypeDef *timer);
    void init();

    int32_t read();
    void write(int32_t count);

    STM32encoder ab;

  private:
};

#endif
