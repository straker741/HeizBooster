#ifndef _ATmegaTimers_H_
#define _ATmegaTimers_H_

#include <inttypes.h>
#include "Arduino.h"

// TC0
#define OC0A_PIN 6
#define OC0B_PIN 5
// TC1
#define OC1A_PIN 9
#define OC1B_PIN 10
// TC2
#define OC2A_PIN 11
#define OC2B_PIN 3


// TOP_AT_FEQUENCY means the TOP value which counter can achieve to function at desired frequency.
static volatile uint8_t TOP_AT_FEQUENCY = 0xFF;


/**
 * Initializes Timer/Counter 2, Fast PWM waveform generation.
 * 
 * @param  f        Desired frequency at the output of OC2B pin.
 * @param  clk_cpu  Frequency at which CPU operates. Default is 16 000 000 Hz.
 * @return          Returns 0 when given frequency is not possible, otherwise 1.
 */
uint8_t PWM_init(float f, uint32_t clk_cpu);


/**
 * Sets OCR2B register based on given percantage as parameter.
 * 
 * @param   p   Floating-point number in percentage.
 * @return      Returns percentage that were saved in OCR2B register.
 */
float PWM_setDutyCycle(float p);


/**
 * Initializes Timer/Counter 1. CTC mode. You must initialize ISR(TIMER1_COMPA_vect).
 * 
 * @param   f       Desired frequency of ISR(TIMER1_COMPA_vect) interrupt service routine.
 * @param  clk_cpu  Frequency at which CPU operates. Default is 16 000 000 Hz.
 * @return          Returns 0 when given frequency is not possible, otherwise 1.
 */
uint16_t TC1_init(float f, uint32_t clk_cpu);


#endif
