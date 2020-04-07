/*
   Copyright (c) 2015,2016,2020 Piotr Stolarz for the Ardiono port

   This software is distributed WITHOUT ANY WARRANTY; without even the
   implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the License for more information.
 */

#include <SPI.h>
#include <nrf_hal.h>

// if 1 - write some info to the serial output
#define INFO_ON_SERIAL   1

// nRF communication channel (1MHz wide)
#define CHANNEL     20

// nRF Chip Enable (CE) pin
#define CE_PIN      9
// SPI CS (slave select) pin
#define CS_PIN      10
// blinking LED pin
#define LED_PIN     8

#define chip_enable()   digitalWrite(CE_PIN, HIGH)
#define chip_disable()  digitalWrite(CE_PIN, LOW)

static uint8_t rx[NRF_MAX_PL] = {};
static int cnt = 0;
static uint8_t led_val = LOW;

#define toggle_led() \
    digitalWrite(LED_PIN, (led_val==LOW ? (led_val=HIGH) : (led_val=LOW)))


void setup()
{
    // CE as output
    pinMode(CE_PIN, OUTPUT);
    chip_disable()

    // init LED
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, led_val);

#if INFO_ON_SERIAL
    Serial.begin(115200);
#endif

    // SPI init
    SPI.begin();
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));

    // init NRF HAL
    hal_nrf_set_cs_pin(CS_PIN);

    restart_rx();

#if INFO_ON_SERIAL
    uint8_t addr[5];
    char sp_buf[48];

    hal_nrf_get_address(HAL_NRF_PIPE0, addr);
    sprintf(sp_buf, "P0 addr: %02x:%02x:%02x:%02x:%02x",
        addr[0], addr[1], addr[2], addr[3], addr[4]);
    Serial.println(sp_buf);

    Serial.println("Tuned up, waiting for messages...");
#endif
}

/* RX loop */
void loop()
{
    uint8_t irq_flg;

    irq_flg = hal_nrf_get_clear_irq_flags();
    if (irq_flg & (1U<<HAL_NRF_RX_DR))
    {
        /* read RX FIFO of received messages */
        while (!hal_nrf_rx_fifo_empty())
        {
            hal_nrf_read_rx_payload(rx);
            if (rx[0]==0xAB)
            {
                toggle_led();
#if INFO_ON_SERIAL
                Serial.print("Received: \"");
                Serial.print((const char *)&rx[1]);
                Serial.println("\"");
#endif
            }
            cnt = 0;
        }
    }

    // for unknown reason, from time to time, RX seems to be locking in and
    // stops to detect incoming traffic until restarting the transceiver
    if (cnt >= 5000)
    {
        cnt = 0;
        restart_rx();
    } else {
        delayMicroseconds(1000);
        cnt++;
    }
}

void init_rx()
{
    hal_nrf_flush_rx();
    hal_nrf_flush_tx();

    // clear all extra features for the purpose of this example
    hal_nrf_enable_dynamic_payload(false);
    hal_nrf_enable_ack_payload(false);
    hal_nrf_enable_dynamic_ack(false);

    hal_nrf_set_rf_channel(CHANNEL);
    hal_nrf_set_datarate(HAL_NRF_1MBPS);
    hal_nrf_set_crc_mode(HAL_NRF_CRC_16BIT);

    // RX setup: use default pipe 0 address
    hal_nrf_set_operation_mode(HAL_NRF_PRX);

    hal_nrf_close_pipe(HAL_NRF_ALL);
    hal_nrf_config_rx_pipe(HAL_NRF_PIPE0, NULL, true, NRF_MAX_PL);

    // TX output power for auto ack
    hal_nrf_set_output_power(HAL_NRF_0DBM);
}

void restart_rx()
{
    chip_disable();

    hal_nrf_set_power_mode(HAL_NRF_PWR_DOWN);
    delayMicroseconds(1000);

    hal_nrf_set_power_mode(HAL_NRF_PWR_UP);
    delayMicroseconds(1500);

    init_rx();

    chip_enable();
    delayMicroseconds(150);
}
