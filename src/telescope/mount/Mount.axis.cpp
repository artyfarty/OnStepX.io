//--------------------------------------------------------------------------------------------------
// telescope mount control, axis instances

#include "Mount.h"

#ifdef MOUNT_PRESENT

#ifdef AXIS1_ODRIVE_PRESENT
  const ODriveDriverSettings ODriveSettingsAxis1 = {AXIS1_DRIVER_MODEL, AXIS1_DRIVER_STATUS};
  ODriveMotor motor1(1, &ODriveSettingsAxis1);
#endif
#ifdef AXIS1_SERVO_PRESENT
  ServoControl servoControlAxis1;

  #if AXIS1_SERVO_ENCODER == ENC_AB
    Encoder encAxis1(AXIS1_SERVO_ENC1_PIN, AXIS1_SERVO_ENC2_PIN);
  #else
    Encoder encAxis1(AXIS1_SERVO_ENC1_PIN, AXIS1_SERVO_ENC2_PIN, AXIS1_SERVO_ENCODER, AXIS1_SERVO_ENCODER_TRIGGER, &servoControlAxis1.directionHint);
  #endif

  #if AXIS1_SERVO_FEEDBACK == FB_PID
    Pid pidAxis1(AXIS1_SERVO_P, AXIS1_SERVO_I, AXIS1_SERVO_D, AXIS1_SERVO_P_GOTO, AXIS1_SERVO_I_GOTO, AXIS1_SERVO_D_GOTO);
  #endif

  const ServoDriverPins ServoPinsAxis1 = {AXIS1_SERVO_PH1_PIN, AXIS1_SERVO_PH1_STATE, AXIS1_SERVO_PH2_PIN, AXIS1_SERVO_PH2_STATE, AXIS1_ENABLE_PIN, AXIS1_ENABLE_STATE, AXIS1_FAULT_PIN};
  const ServoDriverSettings ServoSettingsAxis1 = {AXIS1_DRIVER_MODEL, AXIS1_DRIVER_STATUS};
  ServoMotor motor1(1, &ServoPinsAxis1, &ServoSettingsAxis1, &encAxis1, &pidAxis1, &servoControlAxis1);
#endif
#ifdef AXIS1_DRIVER_PRESENT
  const StepDirDriverPins StepDirPinsAxis1 = {AXIS1_STEP_PIN, AXIS1_STEP_STATE, AXIS1_DIR_PIN, AXIS1_ENABLE_PIN, AXIS1_ENABLE_STATE, AXIS1_M0_PIN, AXIS1_M1_PIN, AXIS1_M2_PIN, AXIS1_M2_ON_STATE, AXIS1_M3_PIN, AXIS1_DECAY_PIN, AXIS1_FAULT_PIN};
  const StepDirDriverSettings StepDirSettingsAxis1 = {AXIS1_DRIVER_MODEL, AXIS1_DRIVER_MICROSTEPS, AXIS1_DRIVER_MICROSTEPS_GOTO, AXIS1_DRIVER_IHOLD, AXIS1_DRIVER_IRUN, AXIS1_DRIVER_IGOTO, AXIS1_DRIVER_DECAY, AXIS1_DRIVER_DECAY_GOTO, AXIS1_DRIVER_STATUS};
  StepDirMotor motor1(1, &StepDirPinsAxis1, &StepDirSettingsAxis1);
#endif

const AxisPins PinsAxis1 = {AXIS1_SENSE_LIMIT_MIN_PIN, AXIS1_SENSE_HOME_PIN, AXIS1_SENSE_LIMIT_MAX_PIN, {AXIS1_SENSE_HOME, AXIS1_SENSE_HOME_INIT, degToRadF(AXIS1_SENSE_HOME_DIST_LIMIT), AXIS1_SENSE_LIMIT_MIN, AXIS1_SENSE_LIMIT_MAX, AXIS1_SENSE_LIMIT_INIT}};
const AxisSettings SettingsAxis1 = {AXIS1_STEPS_PER_DEGREE*RAD_DEG_RATIO, AXIS1_REVERSE, {degToRadF(AXIS1_LIMIT_MIN), degToRadF(AXIS1_LIMIT_MAX)}, siderealToRad(TRACK_BACKLASH_RATE)};
Axis axis1(1, &PinsAxis1, &SettingsAxis1, AXIS_MEASURE_RADIANS);

#ifdef AXIS2_ODRIVE_PRESENT
  const ODriveDriverSettings ODriveSettingsAxis2 = {AXIS2_DRIVER_MODEL, AXIS2_DRIVER_STATUS};
  StepDirMotor motor2(2, &Serial3, &ODriveSettingsAxis2);
#endif
#ifdef AXIS2_SERVO_PRESENT
  ServoControl servoControlAxis2;

  #if AXIS2_SERVO_ENCODER == ENC_AB
    Encoder encAxis2(AXIS2_SERVO_ENC1_PIN, AXIS2_SERVO_ENC2_PIN);
  #else
    Encoder encAxis2(AXIS2_SERVO_ENC1_PIN, AXIS2_SERVO_ENC2_PIN, AXIS2_SERVO_ENCODER, AXIS2_SERVO_ENCODER_TRIGGER, &servoControlAxis2.directionHint);
  #endif

  #if AXIS2_SERVO_FEEDBACK == FB_PID
    Pid pidAxis2(AXIS2_SERVO_P, AXIS2_SERVO_I, AXIS2_SERVO_D, AXIS2_SERVO_P_GOTO, AXIS2_SERVO_I_GOTO, AXIS2_SERVO_D_GOTO);
  #endif

  const ServoDriverPins ServoPinsAxis2 = {AXIS2_SERVO_PH1_PIN, AXIS2_SERVO_PH1_STATE, AXIS2_SERVO_PH2_PIN, AXIS2_SERVO_PH2_STATE, AXIS2_ENABLE_PIN, AXIS2_ENABLE_STATE, AXIS2_FAULT_PIN};
  const ServoDriverSettings ServoSettingsAxis2 = {AXIS2_DRIVER_MODEL, AXIS2_DRIVER_STATUS};
  ServoMotor motor2(2, &ServoPinsAxis2, &ServoSettingsAxis2, &encAxis2, &pidAxis2, &servoControlAxis2);
  IRAM_ATTR void moveAxis2() { motor2.move(); }
#endif
#ifdef AXIS2_DRIVER_PRESENT
  const StepDirDriverPins StepDirPinsAxis2 = {AXIS2_STEP_PIN, AXIS2_STEP_STATE, AXIS2_DIR_PIN, AXIS2_ENABLE_PIN, AXIS2_ENABLE_STATE, AXIS2_M0_PIN, AXIS2_M1_PIN, AXIS2_M2_PIN, AXIS2_M2_ON_STATE, AXIS2_M3_PIN, AXIS2_DECAY_PIN, AXIS2_FAULT_PIN};
  const StepDirDriverSettings StepDirSettingsAxis2 = {AXIS2_DRIVER_MODEL, AXIS2_DRIVER_MICROSTEPS, AXIS2_DRIVER_MICROSTEPS_GOTO, AXIS2_DRIVER_IHOLD, AXIS2_DRIVER_IRUN, AXIS2_DRIVER_IGOTO, AXIS2_DRIVER_DECAY, AXIS2_DRIVER_DECAY_GOTO, AXIS2_DRIVER_STATUS};
  StepDirMotor motor2(2, &StepDirPinsAxis2, &StepDirSettingsAxis2);
#endif
const AxisPins PinsAxis2 = {AXIS2_SENSE_LIMIT_MIN_PIN, AXIS2_SENSE_HOME_PIN, AXIS2_SENSE_LIMIT_MAX_PIN, {AXIS2_SENSE_HOME, AXIS2_SENSE_HOME_INIT, degToRadF(AXIS2_SENSE_HOME_DIST_LIMIT), AXIS2_SENSE_LIMIT_MIN, AXIS2_SENSE_LIMIT_MAX, AXIS2_SENSE_LIMIT_INIT}};
const AxisSettings SettingsAxis2 = {AXIS2_STEPS_PER_DEGREE*RAD_DEG_RATIO, AXIS2_REVERSE, {degToRadF(AXIS2_LIMIT_MIN), degToRadF(AXIS2_LIMIT_MAX)}, siderealToRad(TRACK_BACKLASH_RATE)};
Axis axis2(2, &PinsAxis2, &SettingsAxis2, AXIS_MEASURE_RADIANS);

#endif
