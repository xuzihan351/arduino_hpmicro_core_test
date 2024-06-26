/*
 * Copyright (c) 2024 HPMicro
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
*/

#ifndef Pins_Arduino_h
#define Pins_Arduino_h
#include "board.h"
#include "hpm_adc16_drv.h"
#include "hpm_gpio_regs.h"
#include "hpm_clock_drv.h"
#include "hpm_gptmr_drv.h"
#include "port/hpm_types.h"
#include "class/ring_buffer.h"

#ifdef __cplusplus
extern "C"{
#endif
#define NUM_DIGITAL_PINS            20
#define NUM_ANALOG_INPUTS           6
#define analogInputToDigitalPin(p)  ((p < 6) ? (p) + 14 : -1)

#if defined(__AVR_ATmega8__)
#define digitalPinHasPWM(p)         ((p) == 9 || (p) == 10 || (p) == 11)
#else
#define digitalPinHasPWM(p)         ((p) == 3 || (p) == 5 || (p) == 6 || (p) == 9 || (p) == 10 || (p) == 11)
#endif

#define LED_BUILTIN   PIN_LED_BUILTIN
#define PIN_SPI_SS    PIN_J3_24
#define PIN_SPI_MOSI  PIN_J3_19
#define PIN_SPI_MISO  PIN_J3_21
#define PIN_SPI_SCK   PIN_J3_23
#define PIN_PWM       PIN_J3_33
#define PIN_ADC       PIN_J3_3

typedef enum {
    PIN_LED_BUILTIN = 0,
    PIN_LED_R = 0,
    PIN_LED_G = 1,
    PIN_PUSH_BUTTON = 2,
    PIN_LED_B = 3,
    PIN_CONSOLE_RX = 4,
    PIN_CONSOLE_TX = 5,
    PIN_J3_3, /* PB09, ADC0.IN1 */
    PIN_J3_5, /* PB08, ADC0.IN11 */
    PIN_J3_7, /* PA02 */
    PIN_J3_8, /* PB15, ADC0.IN7 */
    PIN_J3_10, /* PB14, ADC0.IN6 */
    PIN_J3_11, /* PA31 */
    PIN_J3_13, /* PB11, ADC0.IN3 */
    PIN_J3_19, /* PA29 */
    PIN_J3_21, /* PA28 */
    PIN_J3_23, /* PA27 */
    PIN_J3_24, /* PA26 */
    PIN_J3_26, /* PB10, ADC0.IN2 */
    PIN_J3_27, /* PB12, ADC0.IN4 */
    PIN_J3_28, /* PB13, ADC0.IN5 */
    PIN_J3_29, /*  */
    PIN_J3_31, /*  */
    PIN_J3_32, /* PA09 */
    PIN_J3_33, /* PA10 */
    PIN_J3_36, /* PA00 */
    PIN_J3_37, /* PA30 */
    PIN_J3_38, /* PA01 */
    PIN_MAX_NUM,
} pin_index_t;
typedef int pin_size_t;

#define BOARD_NR_GPIO_PINS         PIN_MAX_NUM
#define BOARD_NR_PWM_PINS          2
#define BOARD_NR_ADC_PINS          8

static const uint8_t LED_R   = PIN_LED_R;
static const uint8_t LED_G   = PIN_LED_G;
static const uint8_t LED_B   = PIN_LED_B;
// Arduino compatible A0-A5 defines:
#define A0 16
#define A1 17
#define A2 18
#define A3 19
#define A4 20
#define A5 21

#define USART_RX_BUF_SIZE  (128)
#define USART_TX_BUF_SIZE  (128)

#define GPIO0_A_IRQ_GROUP    0
#define GPIO0_B_IRQ_GROUP    1
#define GPIO0_X_IRQ_GROUP    2
#define GPIO0_Y_IRQ_GROUP    3

typedef struct usart_dev{
    UART_Type* base;
    clock_name_t clock_name;
    uint8_t tx_buf[USART_TX_BUF_SIZE];
    uint8_t rx_buf[USART_RX_BUF_SIZE];
    ring_buffer *rb;                 /**< RX ring buffer */
    ring_buffer *wb;                 /**< TX ring buffer */
    uint32_t irq_num;
    uint32_t tx_ioc_idx;
    uint32_t tx_ioc_fun;
    bool tx_pioc;
    uint32_t tx_pioc_fun;

    uint32_t rx_ioc_idx;
    uint32_t rx_ioc_fun;
    bool rx_pioc;
    uint32_t rx_pioc_fun;

    pin_size_t tx_pin;
    pin_size_t rx_pin;
} uart_dev_t;
typedef struct _pwm_info {
    GPTMR_Type *base;
    clock_name_t clock;
    uint32_t pin;
    uint32_t ch;
    uint32_t reload;
    uint32_t irq_num;
    uint32_t ioc_idx;
    uint32_t ioc_fun;
    bool pioc;
    uint32_t pioc_idx;
    uint32_t pioc_fun;
    uint32_t irq_group;
} pwm_info_t;

typedef struct gpio_isr_info {
    voidFuncPtrParam handler;
    void *arg;
} gpio_isr_info_t;
extern gpio_isr_info_t gpio_handlers[8][32];
typedef struct _adc_info {
    ADC16_Type* base;
    uint32_t ch;
    clock_name_t clock_name;
    uint32_t irq_num;
    uint32_t sample_cycle;
    const char *name;
    uint32_t ioc_idx;
    uint32_t ioc_fun;
    bool pioc;
    uint32_t pioc_idx;
    uint32_t pioc_fun;
} adc_info_t;
extern uart_dev_t uart_console;
extern uart_dev_t uart_console2;

typedef struct {
    GPIO_Type* ctrl;
    uint8_t oe;
    uint8_t do_idx;
    uint8_t di_idx;
    uint8_t pin;
    uint32_t ioc_idx;
    uint32_t ioc_fun;
    uint32_t irq_num;
    bool pioc;
    uint32_t pioc_fun;
    bool bioc;
    uint32_t bioc_fun;
    uint32_t irq_group;
} gpio_desc_t;

typedef struct spi_dev
{
    SPI_Type *spi;
    uint8_t mode;
    clock_name_t clock_name;
    uint32_t irq_num;
    uint16_t cs_ioc_pad;
    uint32_t cs_ioc_func;
    uint16_t clk_ioc_pad;
    uint32_t clk_ioc_func;
    uint16_t miso_ioc_pad;
    uint32_t miso_ioc_func;
    uint16_t mosi_ioc_pad;
    uint32_t mosi_ioc_func;
    uint16_t io2_ioc_pad;
    uint32_t io2_ioc_func;
    uint16_t io3_ioc_pad;
    uint32_t io3_ioc_func;
    uint8_t status;
    voidFuncPtrParam isr_handler;
    void *isr_arg;
} spi_dev_t;

extern spi_dev_t spi1_dev;

typedef struct i2c_dev
{
    I2C_Type *i2c;
    clock_name_t clock_name;
    uint32_t irq_num;
    uint16_t scl_ioc_pad;
    uint32_t scl_ioc_func;
    uint32_t scl_ioc_pad_ctrl;
    uint16_t sda_ioc_pad;
    uint32_t sda_ioc_func;
    uint32_t sda_ioc_pad_ctrl;
    bool is_master;
    uint8_t status;
    voidFuncPtr isr_handler;
    void *isr_arg;
} i2c_dev_t;
extern i2c_dev_t i2c2_dev;

/**
 * @brief Stores HPMicro-specific information related to a given Maple pin.
 * @see PIN_MAP
 */
typedef struct hpm_pin_info {
    const gpio_desc_t *gpio_device;      /**< Maple pin's GPIO device */
    const pwm_info_t *pwm_device;    /**< Pin's timer device, if any. */
    const adc_info_t *adc_device;        /**< ADC device, if any. */
} hpm_pin_info_t;

/**
 * @brief Maps each Maple pin to a corresponding hpmicro pin info.
 * @see hpm_pin_info_t
 */
extern const hpm_pin_info_t PIN_MAP[BOARD_NR_GPIO_PINS];
/**
 * @brief Pins capable of PWM output.
 *
 * Its length is BOARD_NR_PWM_PINS.
 */
extern const uint8 boardPWMPins[];

/**
 * @brief Array of pins capable of analog input.
 *
 * Its length is BOARD_NR_ADC_PINS.
 */
extern const uint8 boardADCPins[];

/**
 * @brief Pins which are connected to external hardware.
 *
 * For example, on Maple boards, it always at least includes
 * PB1 for the LED.  Its length is BOARD_NR_USED_PINS.
 */
extern const uint8 boardUsedPins[];

extern const pwm_info_t pwm_info[];
extern const adc_info_t board_adc_infos[];

#ifdef __cplusplus
} // extern "C"
#endif
#endif
