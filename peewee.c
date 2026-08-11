 /*************************************************************************

   peewee the penguin, solar bird

   attiny85 power experiments

   28.9.08 christoph(at)roboterclub-freiburg.de
   08.9.15 urbanbieri(at)gmx.ch
   07.29.26 wuntingchan(at)protonmail.com
   2026-07-30 homemade(at)wenzellabs.de - attiny85 port for FOSDEM-85 HW

*************************************************************************/
/*************************************************************************

   Hardware

   prozessor:  ATtin85
   clock:      16.5 Mhz internal oscillator

   PIN5, PORTB0/OC10: piezo speaker

*************************************************************************/
#include <avr/io.h>
#include <avr/power.h>

#define PIEZOSPEAKER     (1<<PINB0)

#define SPEAKEROFF  do {TCCR0A=(0x02);} while(0) // PIN to normal port operation
#define SPEAKERON   do {TCCR0A=((1<<COM0A0) | 0x02);} while (0) // toggle PIN

// random pauses settings
#define SHORT_PAUSE_MIN 1000u
#define SHORT_PAUSE_MAX 7000u

#define LONG_PAUSE_MIN 15000u
#define LONG_PAUSE_MAX 60000u

#define GROUP_PAUSE_MIN 30000u
#define GROUP_PAUSE_MAX 65000u

#define CHIRPS_PER_GROUP_MIN 5u
#define CHIRPS_PER_GROUP_MAX 14u

// random pauses variables
uint16_t random_state = 0xACE1u;  // will be overwritten by seed_rng()

// Seed the LFSR from ADC noise (internal temperature sensor on ATtiny85).
// Each conversion contributes one bit: bit0 XOR bit1 of the ADC result.
// Guarded so the code still builds for ATtiny13 (uses compile-time seed).
static void seed_rng(void)
{
#ifdef __AVR_ATtiny85__
    uint8_t  i;
    uint16_t seed = 0;

    // Internal 1.1 V reference (REFS1=1, REFS0=0), temperature sensor MUX=0x0F
    ADMUX  = (1 << REFS1) | 0x0Fu;
    // Enable ADC, prescaler /128  →  16.5 MHz/128 ≈ 129 kHz (within 50–200 kHz spec)
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

    // Discard the first conversion (reference and mux need settling time)
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));

    // Collect 16 bits: XOR the two LSBs of each result, shift seed by one
    for (i = 0; i < 16u; i++) {
        uint16_t adc_val;
        ADCSRA |= (1 << ADSC);
        while (ADCSRA & (1 << ADSC));
        adc_val = ADC;
        seed = (uint16_t)(seed << 1) | ((adc_val ^ (adc_val >> 1)) & 0x01u);
    }

    // Disable ADC to save power
    ADCSRA &= ~(1 << ADEN);

    // 0x0000 is the only state that locks a Galois LFSR forever; fall back
    // to the compile-time constant when the noise collapsed to all zeros.
    random_state = (seed != 0x0000u) ? seed : 0xACE1u;
#endif
}

// global variables
uint8_t tone_global = 101;
uint16_t delay_global = 16;
uint8_t cycles_global = 10;

void init_timer(){
    seed_rng();

#ifdef __AVR_ATtiny85__
   // we're comming from a 1.2MHz ATtiny13 and now run on a
   // 16.5MHz ATtiny85 so we scale down. x16 is closest
   clock_prescale_set(clock_div_16); // scale ATtiny85 16.5MHz down to ~1.03MHz (closer to original 1.2MHz of ATtiny13)
#endif

   DDRB |= PIEZOSPEAKER; // piezo as output

   TCCR0A=(1<<COM0A0) | 0x02; //CTC mode and toogle OC0A port on compare match
   TCCR0B=(1<<CS00) ; // no prescaling
   OCR0A=255; // in CTC Mode the counter counts up to OCR0A
}

// unspecified delay time
void delay_(uint16_t duration){
   uint16_t d;
   uint16_t n;
   uint16_t counter;

   counter=(duration);
   for(d=0;d<counter;d++){
      for(n=0;n<3;n++) PORTB &= ~(PIEZOSPEAKER);
      for(n=0;n<3;n++) PORTB &= ~(PIEZOSPEAKER);
   }
}

// 16-bit pseudo random num generator
uint16_t random16(void)
{
    uint8_t least_significant_bit;

    least_significant_bit = random_state & 1u;
    random_state >>= 1;

    if (least_significant_bit) {
        random_state ^= 0xB400u;
    }

    return random_state;
}

// Return a random number from minimum through maximum, inclusive.
uint16_t random_range(uint16_t minimum, uint16_t maximum)
{
    return minimum +
           (random16() % ((maximum - minimum) + 1u));
}


void glissando(uint8_t start, uint8_t stop, uint16_t duration){
    uint8_t i;
    if(start <= stop){
        for (i = start; i < stop; i++){
            OCR0A = i;
            delay_(duration);
        }

    } else {
        for(i = start; i > stop; i--){
            OCR0A = i;
            delay_(duration);
        }
    }
}

void chirp(uint8_t tone, uint16_t delay){

    // das erste Glissando ist langsamer:
    // das Delay ist länger
    glissando(tone, tone-60, delay*3);

    SPEAKEROFF;
    delay_(1000);

    SPEAKERON;
    glissando(tone-60, tone-40, delay);

    SPEAKEROFF;
    delay_(200);

    SPEAKERON;
    glissando(tone-40, tone-10, delay);

    SPEAKEROFF;
    delay_(400);

    SPEAKERON;
    glissando(tone-20, tone+20, delay);
    // halte den letzten Ton
    delay_(delay*64);
}

void playPattern(){

    uint16_t random_pause;

    chirp(tone_global, delay_global);

    random_pause = random_range(
        SHORT_PAUSE_MIN,
            SHORT_PAUSE_MAX
        );

    if ((random16() & 0x07u) == 0u) {
            random_pause = random_range(
                    LONG_PAUSE_MIN,
                    LONG_PAUSE_MAX
            );
    }

    // kurze Pause
    SPEAKEROFF;
    delay_(random_pause);
    SPEAKERON;

    // verändere die Werte
    tone_global = tone_global + 80;
    delay_global = delay_global - 4;
    cycles_global = cycles_global - 1;

    // die Werte zurücksetzen, wenn sie
    // zu gross oder zu klein werden
    if(tone_global > 180) {
    tone_global = tone_global - 190;
    }
    if(delay_global < 8) {
        delay_global = delay_global + 24;
    }

    // immer nach 10 Durchläufen macht der Vogel eine längere Pause
    if (cycles_global == 0){
        SPEAKEROFF;
        delay_(
            random_range(
                GROUP_PAUSE_MIN,
                GROUP_PAUSE_MAX
            )
        );
        SPEAKERON;
        cycles_global = (uint8_t)random_range(
            CHIRPS_PER_GROUP_MIN,
            CHIRPS_PER_GROUP_MAX
        );
    }
}

int main(void)
{
    // Initiation
    init_timer();

    while(1)
    {
        // basic example
        /*
        uint8_t tonhoehe;
        tonhoehe = 122;

        OCR0A = 255;
        delay_(400);

        SPEAKEROFF;
        delay_(9000);

        OCR0A = tonhoehe;
        SPEAKERON;
        delay_(600);
        */

        // advanced sound
        playPattern();

    }
    return 0;
}
/***************************************************************************
*
*   (c) 2008 christoph(at)roboterclub-freiburg.de
*       2015 urbanbieri(at)gmx.ch
*       2026 wuntingchan(at)protonmail.com
*       2026 homemade(at)wenzellabs.de
*
***************************************************************************
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation version 2 of the License,                *
*   If you extend the program please maintain the list of authors.        *
*   If you want to use this software for commercial purposes and you      *
*   don't want to make it open source, please contact the authors for     *
*   licensing.                                                            *
***************************************************************************/
