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

static uint8_t tx[NRF_MAX_PL] = {};
static int cnt = 0;
static uint8_t led_val = LOW;

#define toggle_led() \
    digitalWrite(LED_PIN, (led_val==LOW ? (led_val=HIGH) : (led_val=LOW)))


void setup()
{
    // CE as output
    pinMode(CE_PIN, OUTPUT);
    chip_disable();

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

#if INFO_ON_SERIAL
    init_tx();

    uint8_t addr[5];
    char sp_buf[48];

    hal_nrf_get_address(HAL_NRF_PIPE0, addr);
    sprintf(sp_buf, "P0 addr: %02x:%02x:%02x:%02x:%02x",
        addr[0], addr[1], addr[2], addr[3], addr[4]);
    Serial.println(sp_buf);

    hal_nrf_get_address(HAL_NRF_TX, addr);
    sprintf(sp_buf, "TX addr: %02x:%02x:%02x:%02x:%02x",
        addr[0], addr[1], addr[2], addr[3], addr[4]);
    Serial.println(sp_buf);
#endif
}


/* TX loop */
void loop()
{
    int i;
    uint8_t irq_flg;
    char ibuf[10];

    hal_nrf_set_power_mode(HAL_NRF_PWR_UP);
    delayMicroseconds(1500);

    // although not necessary it's safer to re-initialize TX after
    // transceiver power-up due to observed transceiver lock-ups
    init_tx();

    tx[0] = 0xAB;   // magic
    sprintf(ibuf, "%d", cnt);
    sprintf((char *)&tx[1], "ard-msg no. %s", ibuf);
    hal_nrf_write_tx_payload(tx, sizeof(tx));

    hal_nrf_get_clear_irq_flags();

    // pulse CE for transmission
    chip_enable();
    delayMicroseconds(100);
    chip_disable();

    // wait 2ms for transmission status
    for (i=0; i<20; i++) {
        irq_flg = hal_nrf_get_clear_irq_flags();
        if (irq_flg & ((1U<<HAL_NRF_TX_DS)|(1U<<HAL_NRF_MAX_RT))) break;
        delayMicroseconds(100);
    }

    if ((irq_flg & (1U<<HAL_NRF_MAX_RT)) || i>=20)
    {
        hal_nrf_flush_tx();
#if INFO_ON_SERIAL
        Serial.println("TX timeout");
#endif
    } else {
        toggle_led();
#if INFO_ON_SERIAL
        Serial.print("Sent message no. ");
        Serial.print(cnt, DEC);
        Serial.println();
#endif
    }

    // go sleep up to the next transmission
    hal_nrf_set_power_mode(HAL_NRF_PWR_DOWN);
    delay(1000);

    cnt++;
}

void init_tx()
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

    // TX setup: use default address, max power with no retransmissions
    hal_nrf_set_operation_mode(HAL_NRF_PTX);
    hal_nrf_config_tx(NULL, HAL_NRF_0DBM, 0, 0);
}
