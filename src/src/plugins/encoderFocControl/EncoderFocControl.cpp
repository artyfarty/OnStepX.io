//
// Created by artyfarty on 13.07.2024.
//
#if ENCODER_FOC_CONTROL == ON

#include "EncoderFocControl.h"

#include "../../Common.h"
#include "../../lib/serial/Serial_Local.h"
#include "../../lib/tasks/OnTask.h"

#if ENCODER_FOC_CONTROL_ENCODER_TYPE == AB
#include "../../lib/encoder/quadrature/Quadrature.h"
Quadrature encControlAxis(ENCODER_FOC_CONTROL_ENCODER_PINA, ENCODER_FOC_CONTROL_ENCODER_PINB, ENCODER_FOC_CONTROL_ENCODER_AXIS);
#elif ENCODER_FOC_CONTROL_ENCODER_TYPE == AB_ESP32
#include "../../lib/encoder/quadratureEsp32/QuadratureEsp32.h"
QuadratureEsp32 encControlAxis(SLEW_ENCODER_A_PIN, SLEW_ENCODER_B_PIN, 1);
#elif ENCODER_FOC_CONTROL_ENCODER_TYPE == CW_CCW
#include "../../lib/encoder/cwCcw/CwCcw.h"
CwCcw encControlAxis(SLEW_ENCODER_A_PIN, SLEW_ENCODER_B_PIN, 1);
#elif ENCODER_FOC_CONTROL_ENCODER_TYPE == PULSE_DIR
#include "../../lib/encoder/pulseDir/PulseDir.h"
PulseDir encControlAxis(SLEW_ENCODER_A_PIN, SLEW_ENCODER_B_PIN, 1);
#elif ENCODER_FOC_CONTROL_ENCODER_TYPE == AS37_H39B_B
#include "../../lib/encoder/bissc/As37h39bb.h"
As37h39bb encControlAxis(SLEW_ENCODER_A_PIN, SLEW_ENCODER_B_PIN, 1);
#elif ENCODER_FOC_CONTROL_ENCODER_TYPE == JTW_24BIT
#include "../../lib/encoder/bissc/Jtw24.h"
Jtw24 encControlAxis(SLEW_ENCODER_A_PIN, SLEW_ENCODER_B_PIN, 1);
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
  VF("MSG: encPos = "); VL(newPos);
  if (newPos == INT32_MAX) {
    enFault = true;
    VLF("MSG: Encoder fault");
    newPos = 0;
    return;
  } else enFault = false;

  newPos *= ENCODER_FOC_CONTROL_DIR;

  if (prevPos != newPos) {
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