// Copyright 2022-2024 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>

/* App headers */
#include "platform_conf.h"
#include "platform/app_pll_ctrl.h"
#include "platform/driver_instances.h"
#include "platform/platform_init.h"
#include "adaptive_rate_adjust.h"
#include "usb_support.h"
#include "device_control.h"
#include "device_control.h"

static void mclk_init(chanend_t other_tile_c)
{
#if !appconfEXTERNAL_MCLK && ON_TILE(1)
    app_pll_init();
#endif
#if appconfUSB_ENABLED && ON_TILE(USB_TILE_NO)
    adaptive_rate_adjust_init();
#endif
}

static void flash_init(void)
{
#if ON_TILE(FLASH_TILE_NO)
// Flash fast read is used for reading the WW model in the INT device,
// normal read is used for the DFU in the UA device.
// The two read mechanisms are not compatible, so we must choose them at initialization.
#if !appconfUSB_ENABLED && appconfINTENT_ENABLED
    rtos_qspi_flash_fast_read_init(
            qspi_flash_ctx,
            FLASH_CLKBLK,
            PORT_SQI_CS,
            PORT_SQI_SCLK,
            PORT_SQI_SIO,
            NULL,
            qspi_fast_flash_read_transfer_nibble_swap,
            3,
            QSPI_FLASH_CALIBRATION_ADDRESS);
#else
    fl_QuadDeviceSpec qspi_spec = BOARD_QSPI_SPEC;
    fl_QSPIPorts qspi_ports = {
        .qspiCS = PORT_SQI_CS,
        .qspiSCLK = PORT_SQI_SCLK,
        .qspiSIO = PORT_SQI_SIO,
        .qspiClkblk = FLASH_CLKBLK,
    };

    rtos_dfu_image_init(
            dfu_image_ctx,
            &qspi_ports,
            &qspi_spec,
            1);

    rtos_qspi_flash_init(
            qspi_flash_ctx,
            FLASH_CLKBLK,
            PORT_SQI_CS,
            PORT_SQI_SCLK,
            PORT_SQI_SIO,
            NULL);
#endif
#endif
}

static void gpio_init(void)
{
    static rtos_driver_rpc_t gpio_rpc_config_t0;
    static rtos_driver_rpc_t gpio_rpc_config_t1;
    rtos_intertile_t *client_intertile_ctx[1] = {intertile_ctx};

#if ON_TILE(0)
    rtos_gpio_init(gpio_ctx_t0);

    rtos_gpio_rpc_host_init(
            gpio_ctx_t0,
            &gpio_rpc_config_t0,
            client_intertile_ctx,
            1);

    rtos_gpio_rpc_client_init(
            gpio_ctx_t1,
            &gpio_rpc_config_t1,
            intertile_ctx);
#endif

#if ON_TILE(1)
    rtos_gpio_init(gpio_ctx_t1);

    rtos_gpio_rpc_client_init(
            gpio_ctx_t0,
            &gpio_rpc_config_t0,
            intertile_ctx);

    rtos_gpio_rpc_host_init(
            gpio_ctx_t1,
            &gpio_rpc_config_t1,
            client_intertile_ctx,
            1);
#endif
}

static void i2c_init(void)
{
    static rtos_driver_rpc_t i2c_rpc_config;

#if appconfI2C_CTRL_ENABLED && ON_TILE(I2C_CTRL_TILE_NO)
    rtos_i2c_slave_init(i2c_slave_ctx,
                        (1 << appconfI2C_IO_CORE),
                        PORT_I2C_SCL,
                        PORT_I2C_SDA,
                        appconf_CONTROL_I2C_DEVICE_ADDR);
#endif

#if ON_TILE(I2C_TILE_NO)
    rtos_intertile_t *client_intertile_ctx[1] = {intertile_ctx};
    rtos_i2c_master_init(
            i2c_master_ctx,
            PORT_I2C_SCL, 0, 0,
            PORT_I2C_SDA, 0, 0,
            0,
            100);

    rtos_i2c_master_rpc_host_init(
            i2c_master_ctx,
            &i2c_rpc_config,
            client_intertile_ctx,
            1);
#else
    rtos_i2c_master_rpc_client_init(
            i2c_master_ctx,
            &i2c_rpc_config,
            intertile_ctx);
#endif
}

static void spi_init(void)
{
#if appconfSPI_OUTPUT_ENABLED && ON_TILE(SPI_OUTPUT_TILE_NO)
    rtos_spi_slave_init(spi_slave_ctx,
                        (1 << appconfSPI_IO_CORE),
                        SPI_CLKBLK,
                        SPI_MODE_3,
                        PORT_SPI_SCLK,
                        PORT_SPI_MOSI,
                        PORT_SPI_MISO,
                        PORT_SPI_CS);
#endif
}

#if ON_TILE(1) && appconfRECOVER_MCLK_I2S_APP_PLL
static int *p_lock_status = NULL;
/// @brief Save the pointer to the pll lock_status variable
static void set_pll_lock_status_ptr(int* p)
{
    p_lock_status = p;
}
#endif

static void platform_sw_pll_init(void)
{
#if ON_TILE(1) && appconfRECOVER_MCLK_I2S_APP_PLL

    port_t p_bclk = PORT_I2S_BCLK;
    port_t p_mclk = PORT_MCLK;
    port_t p_mclk_count = PORT_MCLK_COUNT;  // Used internally by sw_pll
    port_t p_bclk_count = PORT_BCLK_COUNT;  // Used internally by sw_pll
    xclock_t ck_bclk = I2S_CLKBLK;

    port_enable(p_mclk);
    port_enable(p_bclk);
    // NOTE:  p_lrclk does not need to be enabled by the caller

    set_pll_lock_status_ptr(&sw_pll.lock_status);
    // Create clock from mclk port and use it to clock the p_mclk_count port which will count MCLKs.
    port_enable(p_mclk_count);
    port_enable(p_bclk_count);

    // Allow p_mclk_count to count mclks
    xclock_t clk_mclk = MCLK_CLKBLK;
    clock_enable(clk_mclk);
    clock_set_source_port(clk_mclk, p_mclk);
    port_set_clock(p_mclk_count, clk_mclk);
    clock_start(clk_mclk);

    // Allow p_bclk_count to count bclks
    port_set_clock(p_bclk_count, ck_bclk);
    sw_pll_init(&sw_pll,
                SW_PLL_15Q16(0.0),
                SW_PLL_15Q16(1.0),
                PLL_CONTROL_LOOP_COUNT_INT,
                PLL_RATIO,
                (appconfBCLK_NOMINAL_HZ / appconfLRCLK_NOMINAL_HZ),
                frac_values_90,
                SW_PLL_NUM_LUT_ENTRIES(frac_values_90),
                APP_PLL_CTL_REG,
                APP_PLL_DIV_REG,
                SW_PLL_NUM_LUT_ENTRIES(frac_values_90) / 2,
                PLL_PPM_RANGE);

    debug_printf("Using SW PLL to track I2S input\n");
    sw_pll_ctx->sw_pll = &sw_pll;
    sw_pll_ctx->p_mclk_count = p_mclk_count;
    sw_pll_ctx->p_bclk_count = p_bclk_count;

#endif
}

static void mics_init(void)
{
    static rtos_driver_rpc_t mic_array_rpc_config;
#if ON_TILE(MICARRAY_TILE_NO)
    rtos_intertile_t *client_intertile_ctx[1] = {intertile_ctx};
    rtos_mic_array_init(
            mic_array_ctx,
            (1 << appconfPDM_MIC_IO_CORE),
            RTOS_MIC_ARRAY_CHANNEL_SAMPLE);
    rtos_mic_array_rpc_host_init(
            mic_array_ctx,
            &mic_array_rpc_config,
            client_intertile_ctx,
            1);
#else
    rtos_mic_array_rpc_client_init(
            mic_array_ctx,
            &mic_array_rpc_config,
            intertile_ctx);
#endif
}

static void i2s_init(void)
{

#if appconfI2S_ENABLED
#if appconfI2S_MODE == appconfI2S_MODE_MASTER
    static rtos_driver_rpc_t i2s_rpc_config;
#endif
#if ON_TILE(I2S_TILE_NO)
#if appconfI2S_MODE == appconfI2S_MODE_MASTER
    rtos_intertile_t *client_intertile_ctx[1] = {intertile_ctx};
    port_t p_i2s_dout[1] = {
            PORT_I2S_DAC_DATA
    };
    port_t p_i2s_din[1] = {
            PORT_I2S_ADC_DATA
    };

    rtos_i2s_master_init(
            i2s_ctx,
            (1 << appconfI2S_IO_CORE),
            p_i2s_dout,
            1,
            p_i2s_din,
            1,
            PORT_I2S_BCLK,
            PORT_I2S_LRCLK,
            PORT_MCLK,
            I2S_CLKBLK);

    rtos_i2s_rpc_host_init(
            i2s_ctx,
            &i2s_rpc_config,
            client_intertile_ctx,
            1);
#elif appconfI2S_MODE == appconfI2S_MODE_SLAVE
    port_t p_i2s_dout[1] = {
            PORT_I2S_ADC_DATA
    };
    port_t p_i2s_din[1] = {
            PORT_I2S_DAC_DATA
    };
    rtos_i2s_slave_init(
            i2s_ctx,
            (1 << appconfI2S_IO_CORE),
            p_i2s_dout,
            1,
            p_i2s_din,
            1,
            PORT_I2S_BCLK,
            PORT_I2S_LRCLK,
            I2S_CLKBLK);
#endif
#else
#if appconfI2S_MODE == appconfI2S_MODE_MASTER
    rtos_i2s_rpc_client_init(
            i2s_ctx,
            &i2s_rpc_config,
            intertile_ctx);
#endif
#endif
#endif
}

static void usb_init(void)
{
#if appconfUSB_ENABLED && ON_TILE(USB_TILE_NO)
    usb_manager_init();
#endif
}

static void uart_init(void)
{
#if appconfINTENT_ENABLED && ON_TILE(UART_TILE_NO)
    hwtimer_t tmr_tx = hwtimer_alloc();

    rtos_uart_tx_init(
            uart_tx_ctx,
            XS1_PORT_1A,    /* J4:24*/
            appconfUART_BAUD_RATE,
            8,
            UART_PARITY_NONE,
            1,
            tmr_tx);
#endif
}
extern device_control_t *device_control_i2c_ctx;

void control_init() {
    control_ret_t ret = CONTROL_SUCCESS;
#if appconfI2C_CTRL_ENABLED && ON_TILE(I2C_TILE_NO)
    ret = device_control_init(device_control_i2c_ctx,
                                DEVICE_CONTROL_HOST_MODE,
                                1,//(NUM_TILE_0_SERVICERS + NUM_TILE_1_SERVICERS - 1), // GPO servicer does not register with I2C or SPI device control context
                                NULL, 0);
    xassert(ret == CONTROL_SUCCESS);

    ret = device_control_start(device_control_i2c_ctx,
                                -1,
                                -1);
    xassert(ret == CONTROL_SUCCESS);
#endif
}

void platform_init(chanend_t other_tile_c)
{
    rtos_intertile_init(intertile_ctx, other_tile_c);
    rtos_intertile_init(intertile_usb_audio_ctx, other_tile_c);
    platform_sw_pll_init();
    mclk_init(other_tile_c);
    gpio_init();
    flash_init();
    i2c_init();
    spi_init();
    mics_init();
    i2s_init();
    usb_init();
    uart_init();
    control_init();
}
