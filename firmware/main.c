/*
 * EBike Smart Controller
 *
 * Copyright (C) Jorge Pinto aka Casainho, 2012, 2013.
 *
 *   casainho [at] gmail [dot] com
 *     www.casainho.net
 *
 * Released under the GPL License, Version 3
 */

#include <stdio.h>
#include <stdint.h>
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_dac.h"
#include "core_cm3.h"
#include "gpio.h"
#include "adc.h"
#include "pwm.h"
#include "bldc.h"
#include "throttle.h"
#include "hall_sensor.h"
#include "dac.h"
#include "config.h"
#include "motor.h"

volatile unsigned int _ms;

void delay_ms (unsigned int ms)
{
  _ms = 1;
  while (ms >= _ms) ;
}

void SysTick_Handler(void) // runs every 1ms
{
  // need to call this every 1ms
  cruise_control_tick ();

  // for delay_ms ()
  _ms++;

  // read throttle value and update PWM duty cycle every 100ms
  update_duty_cycle (throttle_get_percent ()); // 1000 --> 100%
}

void initialize (void)
{
  gpio_init ();
  while (switch_is_set ()) ; // wait
  adc_init ();
  dac_init ();
  brake_init ();
  commutation_disable ();
  pwm_init ();
  hall_sensor_init ();

  /* Setup SysTick Timer for 1 millisecond interrupts, also enables Systick and Systick-Interrupt */
  if (SysTick_Config(SystemCoreClock / 1000))
  {
    /* Capture error */
    while (1);
  }
}

int main (void)
{
  volatile unsigned int throttle_percent = 0;
  volatile unsigned int coast = 1;

  initialize ();

  motor_set_max_current (2); // set max current in amps

  while (1)
  {
      // turn off buffers, so IO occurs immediately
      setvbuf(stdin, NULL, _IONBF, 0);
      setvbuf(stdout, NULL, _IONBF, 0);
      setvbuf(stderr, NULL, _IONBF, 0);

      iprintf("Greetings Earthlings");


    throttle_percent = throttle_get_percent (); // get throttle value
    if (throttle_percent < 50) // if Throttle < 5%
    {
      throttle_percent = 0;
    }

    if (throttle_percent == 0) // coast...
    {
      motor_coast ();
      coast = 1;
    }
    else if (throttle_percent != 0 && coast == 1) // start motor...
    {
      motor_set_duty_cycle (throttle_percent);
      motor_start ();
      coast = 0;
    }
    else // keep motor running...
    {
      motor_set_duty_cycle (throttle_percent);
    }
  }

  // should never arrive here
  return 0;
}
