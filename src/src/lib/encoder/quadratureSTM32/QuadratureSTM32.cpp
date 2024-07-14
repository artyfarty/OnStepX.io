// A/B Quadrature encoders (fast STM32 hardware decode)

#include "QuadratureSTM32.h"

#if AXIS1_ENCODER == AB_STM32 || AXIS2_ENCODER == AB_STM32 || AXIS3_ENCODER == AB_STM32 || \
    AXIS4_ENCODER == AB_STM32 || AXIS5_ENCODER == AB_STM32 || AXIS6_ENCODER == AB_STM32 || \
    AXIS7_ENCODER == AB_STM32 || AXIS8_ENCODER == AB_STM32 || AXIS9_ENCODER == AB_STM32 || \
    (ENCODER_FOC_CONTROL == ON && ENCODER_FOC_CONTROL_ENCODER_TYPE == AB_STM32)

// for example:
// QuadratureSTM32 encoder1(AXIS1_ENCODER_A_PIN, AXIS1_ENCODER_B_PIN, 1);

QuadratureSTM32::QuadratureSTM32(TIM_TypeDef *timer) : ab(timer) {

}

void QuadratureSTM32::init() {
  if (initialized) { VF("WRN: Encoder QuadratureSTM32"); V(axis); VLF(" init(), already initialized!"); return; }

  initialized = true;
}

int32_t QuadratureSTM32::read() {
  if (!initialized) { VF("WRN: Encoder QuadratureSTM32"); V(axis); VLF(" read(), not initialized!"); return 0; }

  count = (int32_t)ab.pos();

  return count + origin;
}

void QuadratureSTM32::write(int32_t count) {
  return;
}

#endif
