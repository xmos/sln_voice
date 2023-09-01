// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <platform.h>

#include "FreeRTOS.h"

#include "platform/app_pll_ctrl.h"
#include "platform/driver_instances.h"
#include "gpio_test/gpio_test.h"

#if XK_VOICE_L71
#define BUTTON_MUTE_BITMASK 0x10
#define BUTTON_BTN_BITMASK  0x20
#define BUTTON_IP_2_BITMASK 0x40
#define BUTTON_IP_3_BITMASK 0x80
#define GPIO_BITMASK    (BUTTON_MUTE_BITMASK | BUTTON_BTN_BITMASK | BUTTON_IP_2_BITMASK | BUTTON_IP_3_BITMASK)
#define GPIO_PORT       PORT_GPI

static int mute_status = -1;

#elif XCOREAI_EXPLORER
#define BUTTON_0_BITMASK    0x01
#define BUTTON_1_BITMASK    0x02
#define GPIO_BITMASK    (BUTTON_0_BITMASK | BUTTON_1_BITMASK)
#define GPIO_PORT       PORT_BUTTONS

#elif OSPREY_BOARD
#define BUTTON_0_BITMASK    0x04
#define GPIO_BITMASK    (BUTTON_0_BITMASK)
#define GPIO_PORT       PORT_BUTTON

#else
#define GPIO_PORT       0
#endif

RTOS_GPIO_ISR_CALLBACK_ATTR
static void gpio_callback(rtos_gpio_t *ctx, void *app_data, rtos_gpio_port_id_t port_id, uint32_t value)
{
    TaskHandle_t task = app_data;
    BaseType_t yield_required = pdFALSE;

    value = (~value) & GPIO_BITMASK;

    xTaskNotifyFromISR(task, value, eSetValueWithOverwrite, &yield_required);

    portYIELD_FROM_ISR(yield_required);
}

static void gpio_handler(rtos_gpio_t *gpio_ctx)
{
    uint32_t value;
    uint32_t gpio_val;

    const rtos_gpio_port_id_t gpio_port = rtos_gpio_port(GPIO_PORT);

    rtos_gpio_port_enable(gpio_ctx, gpio_port);

    rtos_gpio_isr_callback_set(gpio_ctx, gpio_port, gpio_callback, xTaskGetCurrentTaskHandle());
    rtos_gpio_interrupt_enable(gpio_ctx, gpio_port);

    for (;;) {
        xTaskNotifyWait(
                0x00000000UL,    /* Don't clear notification bits on entry */
                0xFFFFFFFFUL,    /* Reset full notification value on exit */
                &value,          /* Pass out notification value into value */
                portMAX_DELAY ); /* Wait indefinitely until next notification */

        gpio_val = rtos_gpio_port_in(gpio_ctx, gpio_port);

#if XK_VOICE_L71
        if (((gpio_val & BUTTON_MUTE_BITMASK) != 0) && (mute_status != 1)) {
            rtos_printf("Mute active\n");
            mute_status = 1;
        } else if (((gpio_val & BUTTON_MUTE_BITMASK) == 0) && (mute_status != 0)) {
            rtos_printf("Mute inactive\n");
            mute_status = 0;
        }

        if ((gpio_val & BUTTON_BTN_BITMASK) == 0) {
            rtos_printf("Button pressed\n");
        }
#elif XCOREAI_EXPLORER
        extern volatile int mic_from_usb;
        extern volatile int aec_ref_source;
        if (( gpio_val & BUTTON_0_BITMASK ) == 0) {
            mic_from_usb = !mic_from_usb;
            rtos_printf("Microphone from USB: %s\n", mic_from_usb ? "true" : "false");
        } else if (( gpio_val & BUTTON_1_BITMASK ) == 0) {
            aec_ref_source = !aec_ref_source;
            rtos_printf("AEC reference source: %s\n", aec_ref_source == appconfAEC_REF_I2S ? "I2S" : "USB");
        }
#elif OSPREY_BOARD
        extern volatile int mic_from_usb;
        if (( gpio_val & BUTTON_0_BITMASK ) == 0) {
            mic_from_usb = !mic_from_usb;
            rtos_printf("Microphone from USB: %s\n", mic_from_usb ? "true" : "false");
        }
#endif
    }
}

extern streaming_channel_t c_ds3;
#define LED_GREEN_MASK      (1<<5)
#define LED_RED_MASK        (1<<4)
#define PDM_OUT_PIN         (1<<7)  //L71 TP10

//alexy porting pcm2pdm
//Tile(0)
//#define PDMOUT_CLKBLK XS1_CLKBLK_1 //XS1_CLKBLK_5
#define PDMOUT_CLK_IN PORT_PDM_CLK_IN
#define PDMOUT_DAT_OUT PORT_PDM_DAT_OUT


extern uint32_t pdm_data[PDM_DATA_SIZE];  // pdm data size for 1 frame (240 samples)
extern uint32_t *p_pdm_data_head;
extern uint32_t *p_pdm_data_tail;
extern uint32_t *p_pdm_data_write;
uint32_t *p_pdm_data_read;

void pdmout_sig(void)
{
    uint32_t value;
    uint32_t gpio_val;
    uint32_t pdmdata=0;
    int32_t i;
    static uint32_t t0=0, t1=0, t2;
    static uint32_t mem_pos = 0;
    port_timestamp_t port_time;
    uint16_t out_time = 0x00;

    //local_thread_mode_set_bits((thread_mode_t) (thread_mode_fast | thread_mode_high_priority));
    rtos_printf("pdmout_sig on tile %d core %d mode %lx\n", THIS_XCORE_TILE, rtos_core_id_get(), local_thread_mode_get_bits());
    clock_enable(PDMOUT_CLKBLK);
    port_reset(PDMOUT_CLK_IN);
    clock_set_source_port(PDMOUT_CLKBLK, PDMOUT_CLK_IN);
    clock_set_divide(PDMOUT_CLKBLK, 0);

    port_start_buffered(PDMOUT_DAT_OUT, 32);
    port_set_clock(PDMOUT_DAT_OUT, PDMOUT_CLKBLK);
    clock_start(PDMOUT_CLKBLK);
    port_clear_buffer(PDMOUT_DAT_OUT);
    //pdmdata = s_chan_in_word(c_ds3.end_b);
    p_pdm_data_read = p_pdm_data_head;

    for (;;) {

        for (mem_pos = 0; mem_pos < 3; mem_pos++) {
            pdmdata = s_chan_in_word(c_ds3.end_b);
            p_pdm_data_read = &(p_pdm_data_head[PDM_DATA_FRAME * mem_pos]);
            for (i=0; i<PDM_DATA_FRAME; i++) {    // 240 sample * (6x); upsample 6 times, 32 bit -> 192 bit 
            out_time += 0x20;
                    port_out_shift_right_at_time(PDMOUT_DAT_OUT, out_time, bitrev(*p_pdm_data_read));            
            p_pdm_data_read++;
            }
        }
    }   
}

void gpio_test(rtos_gpio_t *gpio_ctx)
{
    if (GPIO_PORT != 0) {
        xTaskCreate((TaskFunction_t) gpio_handler,
                    "gpio_handler",
                    RTOS_THREAD_STACK_SIZE(gpio_handler),
                    gpio_ctx,
                    configMAX_PRIORITIES-2,
                    NULL);
    }
}
