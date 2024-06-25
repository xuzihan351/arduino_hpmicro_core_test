/*
 * Copyright (c) 2024 HPMicro
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#include <stdio.h>
#include "board.h"
#include "hpm_debug_console.h"
#include <Arduino.h>
extern void setup(void);
extern void loop(void);
int main()
{
    board_init(); 
    init();
    setup();
    while (1) {
        loop();
    }
    return 0;
}

