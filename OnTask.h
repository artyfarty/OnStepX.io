// --------------------------------------------------------------
// OnTask

#pragma once

// default provision is for 8 tasks, up to 255 are allowed, to change use:
// #define TASKS_MAX 200 (for example) before the #include for this library

// to enable the option to use a given hardware timer (1..4), if available depending on hw) add:
// #define TASKS_HWTIMER1_ENABLE (for example) before the #include for this library

// Normally skipped tasks (too busy) are delayed for later processing, to instead skip them add:
// #define TASKS_SKIP_MISSED

// bring in hardware timer support
#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__)
#include "HAL_ATMEGA328_HWTIMER.h"
#elif defined(__AVR_ATmega1280__) ||defined(__AVR_ATmega2560__)
#include "HAL_MEGA2560_HWTIMER.h"
#elif defined(__TEENSYDUINO__)
#include "HAL_TEENSY_HWTIMER.h"
#else
#include "HAL_EMPTY_HWTIMER.h"
#endif

// up to 8 tasks default
#ifndef TASKS_MAX
#define TASKS_MAX 8
#endif

enum PeriodUnits {PU_NONE, PU_MILLIS, PU_MICROS, PU_SUB_MICROS};

class Task {
  public:
    Task(uint32_t period, uint32_t duration, bool repeat, uint8_t priority, void (*callback)()) {
      this->period = period;
      period_units = PU_MILLIS;
      this->duration = duration;
      this->repeat = repeat;
      this->priority = priority;
      this->callback = callback;
      start_time = millis();
      next_task_time = start_time + period;
    }

    ~Task() {
      switch (hardwareTimer) {
        case 1: HAL_HWTIMER1_DONE(); break;
        case 2: HAL_HWTIMER2_DONE(); break;
        case 3: HAL_HWTIMER3_DONE(); break;
        case 4: HAL_HWTIMER4_DONE(); break;
      }
    }

    bool requestHardwareTimer(uint8_t num, uint8_t hwPriority) {
      if (num < 1 || num > 4) return false;
      if (repeat != true) return false;
      if (priority != 0) return false;

      unsigned long hwPeriod = period;
      if (period_units == PU_NONE)   hwPeriod = 0; else if (period_units == PU_MILLIS) hwPeriod *= 16000UL; else if (period_units == PU_MICROS) hwPeriod *= 16UL;
      HAL_HWTIMER_PREPARE_PERIOD(num, hwPeriod);

      switch (num) {
        case 1:
          if (HAL_HWTIMER1_FUN != NULL) return false;
          HAL_HWTIMER1_SET_PERIOD();
          HAL_HWTIMER1_FUN = callback;
          if (!HAL_HWTIMER1_INIT(hwPriority)) return false;
        break;
        case 2:
          if (HAL_HWTIMER2_FUN != NULL) return false;
          HAL_HWTIMER2_SET_PERIOD();
          HAL_HWTIMER2_FUN = callback;
          if (!HAL_HWTIMER2_INIT(hwPriority)) return false;
        break;
        case 3:
          if (HAL_HWTIMER3_FUN != NULL) return false;
          HAL_HWTIMER3_SET_PERIOD();
          HAL_HWTIMER3_FUN = callback;
          if (!HAL_HWTIMER3_INIT(hwPriority)) return false;
        break;
        case 4:
          if (HAL_HWTIMER4_FUN != NULL) return false;
          HAL_HWTIMER4_SET_PERIOD();
          HAL_HWTIMER4_FUN = callback;
          if (HAL_HWTIMER4_INIT(hwPriority)) return false;
        break;
      }
      hardwareTimer = num;
      period = hwPeriod;
      period_units = PU_SUB_MICROS;
      return true;
    }

    // run task at the prescribed interval
    // priority: level highest 0 to 7 lowest
    // note: tasks are timed in such a way as to achieve an accurate average frequency
    // if the task occurs late the next call is scheduled earlier to make up the difference
    bool poll(uint8_t priority) {
      if (hardwareTimer) return false;

      static uint8_t priority_level_set = 0;
      uint8_t current_priority = 0; bitSet(current_priority, 7 - priority);
      if (priority == this->priority && current_priority > priority_level_set && period > 0) {
        unsigned long t;
        if (period_units == PU_MICROS) t = micros(); else if (period_units == PU_SUB_MICROS) t = micros() * 16; else t = millis();
        unsigned long time_to_next_task = next_task_time - t;
        if ((long)time_to_next_task < 0) {
          bitSet(priority_level_set, 7 - priority);
          callback();
          bitClear(priority_level_set, 7 - priority);

          // adopt next period
          if (next_period_units != PU_NONE) {
            time_to_next_task = 0;
            period = next_period;
            period_units = next_period_units;
            next_period_units = PU_NONE;
          }

#ifdef TASKS_SKIP_MISSED
          // optionally skip missed tasks
          if (-time_to_next_task > period) time_to_next_task = period;
#endif

          // set adjusted period
          next_task_time = t + (long)(period + time_to_next_task);
          if (!repeat) period = 0;
          return true;
        }
      }
      return false;
    }

    void setPeriod(unsigned long period) {
      if (hardwareTimer) {
        next_period = period;
        next_period_units = PU_MILLIS;
        setHardwareTimerPeriod();
      } else {
        if (this->period == 0) {
          this->period = period;
          period_units = PU_MILLIS;
          next_period_units = PU_NONE;
        } else {
          next_period = period;
          next_period_units = PU_MILLIS;
        }
      }
    }

    void setPeriodMicros(unsigned long period) {
      if (hardwareTimer) {
        next_period = period;
        next_period_units = PU_MICROS;
        setHardwareTimerPeriod();
      } else {
        if (this->period == 0) {
          this->period = period;
          period_units = PU_MICROS;
          next_period_units = PU_NONE;
        } else {
          next_period = period;
          next_period_units = PU_MICROS;
        }
      }
    }

    void setPeriodSubMicros(unsigned long period) {
      if (hardwareTimer) {
        next_period = period;
        next_period_units = PU_SUB_MICROS;
        setHardwareTimerPeriod();
      } else {
        if (this->period == 0) {
          this->period = period;
          period_units = PU_SUB_MICROS;
          next_period_units = PU_NONE;
        } else {
          next_period = period;
          next_period_units = PU_SUB_MICROS;
        }
      }
    }

    void setFrequency(double freq) {
      if (freq > 0.0) {
        freq = 1.0 / freq;            // seconds per call
        double x = freq * 16000000.0; // sub-micros per call
        if (x <= 4294967295.0) {
          setPeriodSubMicros(lround(x));
          return;
        }
        x = freq * 1000000.0;         // micros per call
        if (x <= 4294967295.0) {
          setPeriodMicros(lround(x));
          return;
        }
        x = freq * 1000.0;            // micros per call
        if (x <= 4294967295.0) {
          setPeriod(lround(x));
          return;
        }
      }
      setPeriod(0);
    }

    void setDuration(unsigned long duration) {
      this->duration = duration;
    }

    bool isDurationComplete() {
      return (duration > 0 && ((long)(millis() - (start_time + duration)) >= 0));
    }

    void setRepeat(bool repeat) {
      if (hardwareTimer) return;
      this->repeat = repeat;
    }

    void setPriority(bool priority) {
      if (hardwareTimer) return;
      this->priority = priority;
    }

    uint8_t getPriority() {
      return priority;
    }

  private:

    void setHardwareTimerPeriod() {
      // adopt next period
      if (next_period_units != PU_NONE) {
        if (next_period_units == PU_MILLIS) next_period *= 16000UL; else if (next_period_units == PU_MICROS) next_period *= 16UL;
        next_period_units = PU_NONE;
        period_units = PU_SUB_MICROS;
        period = next_period;
        HAL_HWTIMER_PREPARE_PERIOD(hardwareTimer, period);
      }
    }

    uint8_t hardwareTimer = 0;
    unsigned long period = 0;
    unsigned long next_period = 0;
    PeriodUnits period_units = PU_MILLIS;
    PeriodUnits next_period_units = PU_NONE;
    unsigned long start_time = 0;
    unsigned long duration = 0;
    unsigned long next_task_time = 0;
    bool repeat = false;
    uint8_t priority = 0;
    void (*callback)() = NULL;
};

class Tasks {
  public:
    Tasks() {
      // clear tasks
      for (uint8_t c = 0; c < TASKS_MAX; c++) {
        task[c] = NULL;
        allocated[c] = false;
      }
    }

    ~Tasks() {
      // find active tasks and remove
      for (uint8_t c = 0; c < TASKS_MAX; c++) {
        if (allocated[c]) delete task[c];
      }
    }

    // add process task
    // period:   between process calls in milliseconds, use 0 to disable
    //           the possible period in milliseconds is about 49 days
    // duration: in milliseconds the task is valid for, use 0 for unlimited (deletes the task on completion)
    // repeat:   true if the task is allowed to repeat, false to run once (sets period to 0 on completion)
    // priority: level highest 0 to 7 lowest, all tasks of priority 0 must be complete before priority 1 tasks are serviced, etc.
    //           only processes of higher priority are allowed to run while other processes (of a lower priority) are running
    // callback: function to handle this tasks processing
    // returns:  handle to the task on success, or 0 on failure
    uint8_t add(uint32_t period, uint32_t duration, bool repeat, uint8_t priority, void (*callback)()) {

      // check priority
      if (priority > 7) return false;
      if (priority > highest_priority) highest_priority = priority;

      // find the next free task
      int8_t e = -1;
      for (uint8_t c = 0; c < TASKS_MAX; c++) {
        if (!allocated[c]) {
          e = c;
          break;
        }
      }
      // no tasks available
      if (e == -1) return false;

      // create the task handler
      task[e] = new Task(period, duration, repeat, priority, callback);
      allocated[e] = true;

      updateEventRange();

      return e + 1;
    }

    // allocates a hardware timer, if available, for this task
    // handle:       task handle
    // num:          the hardware timer number, there are up to four (1 to 4) depending on the platform
    // hwPriority:   hardware interrupt priority, default is 128
    // returns:      true if successful, or false (in which case the standard polling timer will still be available)
    // notes:
    //   "repeat"   must be true
    //   "priority" must be 0 (all are higher than task priority 0)
    //   generally it's a good idea to NOT poll tasks (P macro) from within the process associated with this task
    bool requestHardwareTimer(uint8_t handle, uint8_t num) {
      return requestHardwareTimer(handle, num, 128);
    }
    bool requestHardwareTimer(uint8_t handle, uint8_t num, uint8_t hwPriority) {
      if (handle != 0 && allocated[handle - 1]) {
        return task[handle - 1]->requestHardwareTimer(num, hwPriority);
      } else return false;
    }

    // remove process task
    // handle: task handle
    // note:   do not remove an task if the task process is running
    void remove(uint8_t handle) {
      if (handle != 0 && allocated[handle - 1]) {
        delete task[handle - 1];
        allocated[handle - 1] = false;
        updateEventRange();
        updatePriorityRange();
      }
    }

    // set process period ms
    // handle: task handle
    // period: in milliseconds, use 0 for disabled
    // notes:
    //   the period/frequency change takes effect on the next task cycle
    //   if the period is > the hardware timers maximum period the task is disabled
    void setPeriod(uint8_t handle, unsigned long period) {
      if (handle != 0 && allocated[handle - 1]) {
        task[handle - 1]->setPeriod(period);
      }
    }

    // set process period us
    // handle: task handle
    // period: in milliseconds, use 0 for disabled
    // notes:
    //   the period/frequency change takes effect on the next task cycle
    //   if the period is > the hardware timers maximum period the task is disabled
    void setPeriodMicros(uint8_t handle, unsigned long period) {
      if (handle != 0 && allocated[handle - 1]) {
        task[handle - 1]->setPeriodMicros(period);
      }
    }

    // set process period sub-us
    // handle: task handle
    // period: in sub-microseconds (1/16 microsecond units), use 0 for disabled
    // notes:
    //   the period/frequency change takes effect on the next task cycle
    //   if the period is > the hardware timers maximum period the task is disabled
    void setPeriodSubMicros(uint8_t handle, unsigned long period) {
      if (handle != 0 && allocated[handle - 1]) {
        task[handle - 1]->setPeriodSubMicros(period);
      }
    }

    // change process period Hz
    // handle: task handle
    // freq:  in Hertz, use 0 for disabled
    // notes:
    //   the period/frequency change takes effect on the next task cycle
    //   when setting a frequency the most appropriate setPeriod is used automatically
    //   if the period is > ~49 days (or > the hardware timers maximum period) the task is disabled
    void setFrequency(uint8_t handle, double freq) {
      if (handle != 0 && allocated[handle - 1]) {
        task[handle - 1]->setFrequency(freq);
      }
    }

    // change process duration (milliseconds,) use 0 for disabled
    void setDuration(uint8_t handle, unsigned long duration) {
      if (handle != 0 && allocated[handle - 1]) {
        task[handle - 1]->setDuration(duration);
      }
    }

    // change process repeat (true/false)
    void setRepeat(uint8_t handle, bool repeat) {
      if (handle != 0 && allocated[handle - 1]) {
        task[handle - 1]->setRepeat(repeat);
      }
    }

    // change process priority level (highest 0 to 7 lowest)
    void setPriority(uint8_t handle, uint8_t priority) {
      if (handle != 0 && allocated[handle - 1]) {
        if (priority > 7) return;
        task[handle - 1]->setPriority(priority);
        updateEventRange();
        updatePriorityRange();
      }
    }

    // runs tasks at the prescribed interval, each call can trigger at most a single process
    // processes that are already running are ignored so it's ok to poll() within a process
    void yield() {
      for (uint8_t priority = 0; priority <= highest_priority; priority++) {
        for (uint8_t i = 0; i <= highest_task; i++) {
          number[priority]++; if (number[priority] > highest_task) number[priority]=0;
          if (allocated[number[priority]]) {
            if (!task[number[priority]]->isDurationComplete()) {
              if (task[number[priority]]->poll(priority)) return;
            } else remove(number[priority] + 1);
          }
        }
      }
    }

  private:
    // keep track of the range of priorities so we don't waste cycles looking at empty ones
    void updatePriorityRange() {
      highest_priority = 0;
      for (uint8_t e = 0; e <= highest_task; e++) {
        uint8_t p = task[e]->getPriority();
        if (p > highest_priority) highest_priority = p;
      }
    }
    // keep track of the range of tasks so we don't waste cycles looking at empty ones
    void updateEventRange() {
      // scan for highest task
      highest_task = 0;
      for (uint8_t e = TASKS_MAX - 1; e >= 0 ; e--) {
        if (allocated[e]) {
          highest_task = e;
          break;
        }
      }
    }

    uint8_t highest_task = 0;
    uint8_t highest_priority = 0;
    uint8_t count = 0;
    uint8_t num_tasks = 0;
    uint8_t number[8] = {255, 255, 255, 255, 255, 255, 255, 255};
    bool    allocated[TASKS_MAX];
    Task *task[TASKS_MAX];
};

Tasks tasks;
#define Y tasks.yield() // short P macro to embed polling
