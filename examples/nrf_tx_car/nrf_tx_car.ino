/*
   Copyright (c) 2020 Piotr Stolarz for the Ardiono port

   This software is distributed WITHOUT ANY WARRANTY; without even the
   implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the License for more information.
 */

/* Transmit a carrier on the CHANNEL. The transmission may be observed via
   'nrf_scan' example.
 */

#include <assert.h>
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

#define chip_enable()   digitalWrite(CE_PIN, HIGH)
#define chip_disable()  digitalWrite(CE_PIN, LOW)

void setup()
{
    // CE as output
    pinMode(CE_PIN, OUTPUT);
    chip_disable();

#if INFO_ON_SERIAL
    Serial.begin(115200);
#endif

    // SPI init
    SPI.begin();
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));

    // init NRF HAL
    hal_nrf_set_cs_pin(CS_PIN);

    hal_nrf_set_power_mode(HAL_NRF_PWR_UP);
    delayMicroseconds(1500);

    hal_nrf_set_operation_mode(HAL_NRF_PTX);

    hal_nrf_enable_continious_wave(true);
    hal_nrf_set_pll_mode(true);
    hal_nrf_set_output_power(HAL_NRF_0DBM);
    hal_nrf_set_rf_channel(CHANNEL);

    assert(hal_nrf_is_continious_wave_enabled() &&
        hal_nrf_get_pll_mode() &&
        (hal_nrf_get_output_power()==HAL_NRF_0DBM) &&
        (hal_nrf_get_rf_channel()==CHANNEL));

#if INFO_ON_SERIAL
    Serial.print("Transmitting RF carrier on channel ");
    Serial.print(CHANNEL, DEC);
    Serial.println("...");
#endif

    chip_enable();
}

void loop()
{
    delay(1000);
}
