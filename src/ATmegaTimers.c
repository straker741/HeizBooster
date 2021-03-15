#include "ATmegaTimers.h"


/**
 * Initializes Timer/Counter 2, Fast PWM waveform generation.
 * 
 * @param  f        Desired frequency at the output of OC2B pin.
 * @param  clk_cpu  Frequency at which CPU operates. Default is 16 000 000 Hz.
 * @return          Returns 0 when given frequency is not possible, otherwise 1.
 */
uint8_t PWM_init(float f, uint32_t clk_cpu) {
    pinMode(OC2B_PIN, OUTPUT);
    TOP_AT_FEQUENCY = ((clk_cpu / 8 ) / f) - 1;
    if (TOP_AT_FEQUENCY > 0xFF) {
        return 0;
    }

    TCCR2A = 0;                              // TC2 Control Register A
    TCCR2B = 0;                              // TC2 Control Register B
    TIMSK2 = 0;                              // TC2 Interrupt Mask Register
    TIFR2 = 0;                               // TC2 Interrupt Flag Register
    TCCR2A |= (1 << COM2B1);                 // Clear OC2B on Compare Match, set OC2B at BOTTOM (non-inverting mode)
    TCCR2A |= (1 << WGM21) | (1 << WGM20);   // Fast PWM (Mode 7)
    TCCR2B |= (1 << WGM22) | (1 << CS21);    // prescaler 8: 16 MHz / 8 == 2 MHz 
    OCR2A = TOP_AT_FEQUENCY;                 // TOP_AT_FEQUENCY means the TOP value which counter can achieve to satisfy required frequency
    OCR2B = 0;                               // P = <BOTTOM, TOP_AT_FEQUENCY>
    return 1;
}


/**
 * Sets OCR2B register based on given percantage as parameter.
 * 
 * @param   p   Floating-point number in percentage.
 * @return      Returns percentage that were saved in OCR2B register.
 */
float PWM_setDutyCycle(float p) {
    uint8_t ocrv = ((TOP_AT_FEQUENCY + 1) * (p / 100.0));
    if (ocrv == 0) {
        OCR2B = TOP_AT_FEQUENCY;
        return 100.0;
    }
    else {
        OCR2B = ocrv - 1;
        return ((float)ocrv / (float)(TOP_AT_FEQUENCY + 1) * 100);
    }
}


/**
 * Initializes Timer/Counter 1. CTC mode. You must initialize ISR(TIMER1_COMPA_vect).
 * 
 * @param   f       Desired frequency of ISR(TIMER1_COMPA_vect) interrupt service routine.
 * @param  clk_cpu  Frequency at which CPU operates. Default is 16 000 000 Hz.
 * @return          Returns 0 when given frequency is not possible, otherwise 1.
 */
uint16_t TC1_init(float f, uint32_t clk_cpu) {
    uint16_t TOP = ((clk_cpu / 64 ) / f) - 1;
    if (TOP > 0xFFFF) {
        return 0;
    }

    //SREG |= (1 << SREG_I);
    TCCR1A = 0;                              // TC1 Control Register A
    TCCR1B = 0;                              // TC1 Control Register B
    TIMSK1 = 0;                              // TC1 Interrupt Mask Register
    TIFR1 = 0;                               // TC1 Interrupt Flag Register

    TIFR1 |= (1 << OCF1A);                   // TC1, Output Compare A Match Flag
    TCCR1B |= (1 << WGM12);                  // CTC (Mode 4)
    TCCR1B |= (1 << CS10) | (1 << CS11);     // prescaler 64:
    OCR1A = TOP;                             // Maximal value a counter can reach before it start counting again from 0
    TIMSK1 = (1 << OCIE1A);                  // TC1, Output Compare A Match Interrupt Enable
    sei();                                   // Enable global interrupt
    TCNT1 = 0;                               // Reset counter

    OCR1B = 0;                               // Not used - set to 0
    return 1;
}
