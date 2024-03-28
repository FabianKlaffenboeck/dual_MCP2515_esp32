#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdlib.h>
#include "esp_attr.h"
#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"
#include <mcp2515.h>
#include <can.h>

#define PIN1_NUM_MISO 19
#define PIN1_NUM_MOSI 23
#define PIN1_NUM_CLK 18
#define PIN1_NUM_CS 21
#define PIN1_NUM_INT 16
#define SPI1_Channel VSPI_HOST
#define PIN2_NUM_MISO 12
#define PIN2_NUM_MOSI 13
#define PIN2_NUM_CLK 14
#define PIN2_NUM_CS 27
#define PIN2_NUM_INT 26
#define SPI2_Channel HSPI_HOST

spi_device_handle_t spi1;
MCP2515 mcp2515_1(&spi1);
spi_device_handle_t spi2;
MCP2515 mcp2515_2(&spi2);

void feedthedog()
{
    TIMERG0.wdt_wprotect = TIMG_WDT_WKEY_VALUE;
    TIMERG0.wdt_feed = 1;
    TIMERG0.wdt_wprotect = 0;
}

void initializeCan1()
{
    spi_bus_config_t buscfg1 = {
        .mosi_io_num = PIN1_NUM_MOSI,
        .miso_io_num = PIN1_NUM_MISO,
        .sclk_io_num = PIN1_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = -1,
        .flags = 0};
    spi_device_interface_config_t devcfg1 = {
        .mode = 0,
        .clock_speed_hz = 16 * 1000 * 1000,
        .spics_io_num = PIN1_NUM_CS,
        .queue_size = 40,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI1_Channel, &buscfg1, SPI_DMA_DISABLED));
    ESP_ERROR_CHECK(spi_bus_add_device(SPI1_Channel, &devcfg1, &spi1));

    mcp2515_1.reset();
    mcp2515_1.setBitrate(CAN_500KBPS, MCP_16MHZ);
    mcp2515_1.setNormalMode();
}
void initializeCan2()
{
    spi_bus_config_t buscfg2 = {
        .mosi_io_num = PIN2_NUM_MOSI,
        .miso_io_num = PIN2_NUM_MISO,
        .sclk_io_num = PIN2_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = -1,
        .flags = 0};

    spi_device_interface_config_t devcfg2 = {
        .mode = 0,
        .clock_speed_hz = 16 * 1000 * 1000,
        .spics_io_num = PIN2_NUM_CS,
        .queue_size = 40,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_Channel, &buscfg2, SPI_DMA_DISABLED));
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_Channel, &devcfg2, &spi2));

    mcp2515_2.reset();
    mcp2515_2.setBitrate(CAN_500KBPS, MCP_16MHZ);
    mcp2515_2.setNormalMode();
}

void CAN1Task(void *parameter)
{
    initializeCan1();
    while (1)
    {
        feedthedog();
        can_frame CANBuff;
        if (mcp2515_1.readMessage(&CANBuff) == MCP2515::ERROR_OK)
        {
            // Do Something with the massage
        }
    }
}

void CAN2Task(void *parameter)
{
    initializeCan2();
    while (1)
    {
        feedthedog();
        can_frame CANBuff;
        if (mcp2515_2.readMessage(&CANBuff) == MCP2515::ERROR_OK)
        {
            // Do Something with the massage
        }
    }
}

extern "C" void app_main()
{
    xTaskCreatePinnedToCore(
        CAN1Task,   /* Function to implement the task */
        "CAN1Task", /* Name of the task */
        10000,      /* Stack size in words */
        NULL,       /* Task input parameter */
        1,          /* Priority of the task */
        NULL,       /* Task handle. */
        0);         /* Core where the task should run */

    xTaskCreatePinnedToCore(
        CAN2Task,   /* Function to implement the task */
        "CAN2Task", /* Name of the task */
        10000,      /* Stack size in words */
        NULL,       /* Task input parameter */
        1,          /* Priority of the task */
        NULL,       /* Task handle. */
        1);         /* Core where the task should run */
}