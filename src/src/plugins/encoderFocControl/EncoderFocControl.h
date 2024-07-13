// Sample plugin
#pragma once

#ifndef ENCODER_FOC_CONTROL
#define ENCODER_FOC_CONTROL OFF
#endif

#ifndef ENCODER_FOC_CONTROL_DIR
#define ENCODER_FOC_CONTROL__DIR 1
#endif

#ifndef ENCODER_FOC_CONTROL_TYPE
#define ENCODER_FOC_CONTROL_TYPE AB
#endif

#ifndef ENCODER_FOC_CONTROL_MULT
#define ENCODER_FOC_CONTROL_MULT 1
#endif

#ifndef ENCODER_FOC_CONTROL_POLLING_RATE_MS
#define ENCODER_FOC_CONTROL_POLLING_RATE_MS 150
#endif

class EncoderFocControl {
public:
    // the initialization method must be present and named: void init();
    void init();

    void loop();

private:

};

extern EncoderFocControl encoderFocControl;
