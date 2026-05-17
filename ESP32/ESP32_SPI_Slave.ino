#include <stdio.h>
#include <string.h>
#include "driver/spi_slave.h"
#include "driver/gpio.h"

#define PIN_MISO 12
#define PIN_MOSI 13
#define PIN_SCLK 14
#define PIN_CS   15

#define RX_SIZE 32

uint8_t rx_buf[7];
uint8_t tx_buf[7]];

void app_main(void)
{
    spi_bus_config_t buscfg = {
        .mosi_io_num = PIN_MOSI,
        .miso_io_num = PIN_MISO,
        .sclk_io_num = PIN_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = RX_SIZE
    };

    spi_slave_interface_config_t slvcfg = {
        .spics_io_num = PIN_CS,
        .flags = 0,
        .queue_size = 1,
        .mode = 2
    };

    spi_slave_initialize(HSPI_HOST, &buscfg, &slvcfg, SPI_DMA_DISABLED);

    while (1)
    {
        spi_slave_transaction_t t;
        memset(&t, 0, sizeof(t));
        memset(rx_buf, 0, sizeof(rx_buf));

        t.length = RX_SIZE * 8;     // length in bits
        t.rx_buffer = rx_buf;
        t.tx_buffer = tx_buf;

        esp_err_t ret = spi_slave_transmit(
            HSPI_HOST,
            &t,
            portMAX_DELAY
        );

        if (ret == ESP_OK)
        {
            printf("Received %d bytes: ", t.trans_len / 8);

            for (int i = 0; i < t.trans_len / 8; i++)
            {
                printf("0x%02X ", rx_buf[i]);
            }

            printf("\n");
        }
    }
}