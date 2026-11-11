#include "led_strip.h"
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/timer.h"
#include "ws2812.pio.h"

typedef struct {
    PIO pio;
    uint sm;
    int num_pixels;
    int block_size;
    int stride;
    int current_pixel;
    int direction;
    uint32_t color;
    uint32_t off;
} kitt_block_state_t;

static uint32_t led_buf[NUM_PIXELS];
static int dma_chan;
static kitt_block_state_t kitt_state;
static repeating_timer_t kitt_timer;

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)(r) << 8) |
           ((uint32_t)(g) << 16) |
           (uint32_t)(b);
}

volatile bool trigger_beat_sound = false;

void init_ws2812_dma(PIO pio, uint sm) {
    dma_chan = dma_claim_unused_channel(true);
    dma_channel_config c = dma_channel_get_default_config(dma_chan);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, false);
    channel_config_set_dreq(&c, pio_get_dreq(pio, sm, true));

    dma_channel_configure(
        dma_chan,
        &c,
        &pio->txf[sm], // Write address (PIO TX FIFO)
        led_buf, // Read address (LED buffer)
        NUM_PIXELS, // Number of transfers
        false // Don't start yet
    );
}

bool kitt_block_dma_callback(repeating_timer_t *rt) {
    kitt_block_state_t *s = (kitt_block_state_t *) rt->user_data;

    // Skip update if previous DMA transfer is still in progress
    if (dma_channel_is_busy(dma_chan)) {
        return true; // Keep the timer running
    }
    
    int max_idx = s->num_pixels - s->block_size;

    // Fill buffer
    for (int i = 0; i < s->num_pixels; i++) {
        if (i >= s->current_pixel && i < s->current_pixel + s->block_size) {
            led_buf[i] = s->color << 8u; // Active
        } else {
            led_buf[i] = s->off << 8u; // Off
        }
    }

    // Start DMA transfer
    dma_channel_set_read_addr(dma_chan, led_buf, true);

    // Advance pixel block and bounce at the ends
    s->current_pixel += s->direction;
    if (s->current_pixel >= max_idx) {
        s->current_pixel = max_idx;
        s->direction = -1; // Reverse direction
        trigger_beat_sound = true;
    } else if (s->current_pixel <= 0) {
        s->current_pixel = 0;
        s->direction = 1; // Forward direction
        trigger_beat_sound = true;
    }

    return true; // Keep the timer running
}

void led_strip_set_bpm(uint32_t bpm) {
    if (bpm == 0) {
        return;
    }

    cancel_repeating_timer(&kitt_timer);

    int steps_per_swing = 2 * (kitt_state.num_pixels - kitt_state.block_size) / kitt_state.stride;
    int64_t step_us = -(60000000LL / (bpm * steps_per_swing));

    add_repeating_timer_us(step_us, kitt_block_dma_callback, &kitt_state, &kitt_timer);
}

void led_strip_init(uint32_t initial_bpm) {
    PIO pio = pio0;
    uint sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, false);

    // Configure DMA channel
    init_ws2812_dma(pio, sm);

    // Initialize kitt block state
    kitt_state = (kitt_block_state_t) {
        .pio = pio,
        .sm = sm,
        .num_pixels = NUM_PIXELS,
        .block_size = 6,
        .stride = 2,
        .current_pixel = 0,
        .direction = 1,
        .color = urgb_u32(255, 0, 0),
        .off = urgb_u32(0, 0, 0)
    };

    led_strip_set_bpm(initial_bpm);
}
