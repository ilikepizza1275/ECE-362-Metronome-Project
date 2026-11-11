#include "pico/stdlib.h"
#include "led_strip.h"

int main() {
    stdio_init_all();

    // Start the LED strip
    led_strip_init(120);

    while (true) {
        if (trigger_beat_sound) {
            trigger_beat_sound = false;
            // Trigger PWM sound effect here
        }

        // UI calls this whenever the user enters a new BPM:
        // led_strip_set_bpm(new_bpm);

        tight_loop_contents();
    }
}