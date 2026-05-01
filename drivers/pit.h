#pragma once

#include <stdint.h>

#define PIT_CRYSTAL 1193182

typedef void (*timer_callback_t)(void);

uint16_t init_pit(uint32_t hz, timer_callback_t callback);
