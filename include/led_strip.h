#ifndef LED_STRIP_H
#define LED_STRIP_H

#include <stdint.h>
#include <stdbool.h>

#define NUM_PIXELS 96 // Number of LEDs on the strip
#define WS2812_PIN 20 // Data pin for the WS2812 strip

// Flag goes true every time the LED block hits either end of the strip.
// The audio loop should check this and reset it to false.
extern volatile bool trigger_beat_sound;

// Initialize the PIO, DMA, and start the sweep at the initial BPM (USE THIS FIRST)
void led_strip_init(uint32_t initial_bpm);

// Call this to change the BPM (ONLY USE AFTER CALLING led_strip_init)
void led_strip_set_bpm(uint32_t bpm);

#endif