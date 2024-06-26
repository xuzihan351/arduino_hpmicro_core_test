
#include "port/hpm_types.h"
#include "arduino/pins_arduino.h"
#include "hpm_pwm_drv.h"

const pwm_info_t pwm_info[PIN_MAX_NUM] = {
    [0] = { /* PIN_PWM_WH */
        .base = HPM_PWM0,
        .clock = clock_mot0,
        .pin = 2,
        .cmp_index = 0,
        .reload = 200000 - 1,
        .irq_num = IRQn_PWM0,
        .ioc_idx = IOC_PAD_PA26,
        .ioc_fun = IOC_PA26_FUNC_CTL_PWM0_P_2,
        .pioc = false,
        .pioc_idx = 0,
        .pioc_fun = 0,
        .irq_group = 0,
    },
    [1] = { /* PIN_PWM_WL */
        .base = HPM_PWM0,
        .clock = clock_mot0,
        .pin = 3,
        .cmp_index = 0,
        .reload = 200000 - 1,
        .irq_num = IRQn_PWM0,
        .ioc_idx = IOC_PAD_PA27,
        .ioc_fun = IOC_PA27_FUNC_CTL_PWM0_P_3,
        .pioc = false,
        .pioc_idx = 0,
        .pioc_fun = 0,
        .irq_group = 0,
    },
    [2] = { /* PIN_PWM_VH */
        .base = HPM_PWM0,
        .clock = clock_mot0,
        .pin = 4,
        .cmp_index = 0,
        .reload = 200000 - 1,
        .irq_num = IRQn_PWM0,
        .ioc_idx = IOC_PAD_PA28,
        .ioc_fun = IOC_PA28_FUNC_CTL_PWM0_P_4,
        .pioc = false,
        .pioc_idx = 0,
        .pioc_fun = 0,
        .irq_group = 0,
    },
    [3] = { /* PIN_PWM_VL */
        .base = HPM_PWM0,
        .clock = clock_mot0,
        .pin = 5,
        .cmp_index = 0,
        .reload = 200000 - 1,
        .irq_num = IRQn_PWM0,
        .ioc_idx = IOC_PAD_PA29,
        .ioc_fun = IOC_PA29_FUNC_CTL_PWM0_P_5,
        .pioc = false,
        .pioc_idx = 0,
        .pioc_fun = 0,
        .irq_group = 0,
    },
    [4] = { /* PIN_PWM_UH */
        .base = HPM_PWM0,
        .clock = clock_mot0,
        .pin = 6,
        .cmp_index = 0,
        .reload = 200000 - 1,
        .irq_num = IRQn_PWM0,
        .ioc_idx = IOC_PAD_PA30,
        .ioc_fun = IOC_PA30_FUNC_CTL_PWM0_P_6,
        .pioc = false,
        .pioc_idx = 0,
        .pioc_fun = 0,
        .irq_group = 0,
    },
    [5] = { /* PIN_PWM_UL */
        .base = HPM_PWM0,
        .clock = clock_mot0,
        .pin = 7,
        .cmp_index = 0,
        .reload = 200000 - 1,
        .irq_num = IRQn_PWM0,
        .ioc_idx = IOC_PAD_PA31,
        .ioc_fun = IOC_PA31_FUNC_CTL_PWM0_P_7,
        .pioc = false,
        .pioc_idx = 0,
        .pioc_fun = 0,
        .irq_group = 0,
    },
};
