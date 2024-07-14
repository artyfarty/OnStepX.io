//
// Created by artyfarty on 13.07.2024.
//
#include "../../Common.h"
#include "../../lib/serial/Serial_Local.h"
#include "../../lib/tasks/OnTask.h"
#include "EncoderFocControl.h"

#if ENCODER_FOC_CONTROL == ON

#if ENCODER_FOC_CONTROL_ENCODER_TYPE == AB
#include "../../lib/encoder/quadrature/Quadrature.h"
Quadrature encControlAxis(ENCODER_FOC_CONTROL_ENCODER_PINA, ENCODER_FOC_CONTROL_ENCODER_PINB, ENCODER_FOC_CONTROL_ENCODER_AXIS);
#elif ENCODER_FOC_CONTROL_ENCODER_TYPE == AB_ESP32
#include "../../lib/encoder/quadratureEsp32/QuadratureEsp32.h"
QuadratureEsp32 encControlAxis(ENCODER_FOC_CONTROL_ENCODER_PINA, ENCODER_FOC_CONTROL_ENCODER_PINB, 1);
#elif ENCODER_FOC_CONTROL_ENCODER_TYPE == AB_STM32
#include "../../lib/encoder/quadratureSTM32/QuadratureSTM32.h"
QuadratureSTM32 encControlAxis(ENCODER_FOC_CONTROL_ENCODER_TIM);
#elif ENCODER_FOC_CONTROL_ENCODER_TYPE == CW_CCW
#include "../../lib/encoder/cwCcw/CwCcw.h"
CwCcw encControlAxis(ENCODER_FOC_CONTROL_ENCODER_PINA, ENCODER_FOC_CONTROL_ENCODER_PINB, 1);
#elif ENCODER_FOC_CONTROL_ENCODER_TYPE == PULSE_DIR
#include "../../lib/encoder/pulseDir/PulseDir.h"
PulseDir encControlAxis(ENCODER_FOC_CONTROL_ENCODER_PINA, ENCODER_FOC_CONTROL_ENCODER_PINB, 1);
#elif ENCODER_FOC_CONTROL_ENCODER_TYPE == AS37_H39B_B
#include "../../lib/encoder/bissc/As37h39bb.h"
As37h39bb encControlAxis(ENCODER_FOC_CONTROL_ENCODER_PINA, ENCODER_FOC_CONTROL_ENCODER_PINB, 1);
#elif ENCODER_FOC_CONTROL_ENCODER_TYPE == JTW_24BIT
#include "../../lib/encoder/bissc/Jtw24.h"
Jtw24 encControlAxis(ENCODER_FOC_CONTROL_ENCODER_PINA, ENCODER_FOC_CONTROL_ENCODER_PINB, 1);
#endif

void pollEncoders() { encoderFocControl.loop(); }

void EncoderFocControl::init() {
  VLF("MSG: Plugins, starting: encoderFocControl");

  encControlAxis.init();
  should_slew = false;

  VF("MSG: Encoders, start polling task (priority 4)... ");
  // start a task that runs twice a second, run at priority level 7 so we can block using tasks.yield(); fairly aggressively without significant impact on operation
  tasks.add(ENCODER_FOC_CONTROL_POLLING_RATE_MS, 0, true, 4, pollEncoders);
}

void EncoderFocControl::loop() {
  long newPos = encControlAxis.read();
  if (newPos == INT32_MAX) {
    enFault = true;
    VLF("MSG: Encoder fault");
    newPos = 0;
    return;
  } else enFault = false;

  newPos *= ENCODER_FOC_CONTROL_DIR;

  if (prevPos != newPos) {
    VF("MSG: encPos = "); VL(newPos);
    should_slew = true;
    posDiff = newPos - prevPos;
    prevPos = newPos;
  } else {
    should_slew = false;
  }

  if (should_slew) {
    // // :FR[sn]#   Goto focuser target position relative (in microns or steps)
    sprintf(fp_cmd, ":FR%ld", posDiff * ENCODER_FOC_CONTROL_MULT);

    SERIAL_LOCAL.transmit(fp_cmd);
    VF("MSG: FocusPull "); VL(posDiff);
  }
}

EncoderFocControl encoderFocControl;

#endif