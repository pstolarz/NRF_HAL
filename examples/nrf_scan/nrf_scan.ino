/*
   Copyright (c) 2020 Piotr Stolarz for the Ardiono port

   This software is distributed WITHOUT ANY WARRANTY; without even the
   implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the License for more information.
 */

/* Simple 2.4GHz channels scanning example.
 */

#include <assert.h>
#include <SPI.h>
#include <nrf_hal.h>

// first and last channel to scan
#define CHANNEL_START   0
#define CHANNEL_STOP    127

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

    Serial.begin(115200);

    // SPI init
    SPI.begin();
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));

    // init NRF HAL
    hal_nrf_set_cs_pin(CS_PIN);

    hal_nrf_set_power_mode(HAL_NRF_PWR_UP);
    delayMicroseconds(1500);

    hal_nrf_set_operation_mode(HAL_NRF_PRX);
}

void loop()
{
    int i;
    bool eol=false;

    Serial.print("Scanning starts at ");
    Serial.print(CHANNEL_START, DEC);
    Serial.print(" , ends at ");
    Serial.print(CHANNEL_STOP, DEC);
    Serial.println(" channel...");

    for (int ch=CHANNEL_START; ch<=CHANNEL_STOP; ch++)
    {
        if (eol) Serial.println();

        hal_nrf_set_rf_channel(ch);
        assert(hal_nrf_get_rf_channel()==ch);

        chip_enable();

        for (i=0; i<500; i++) {
            if (hal_nrf_get_carrier_detect()) break;
            delayMicroseconds(1000);
        }

        if (i<500) {
            if (!eol) Serial.println();
            Serial.print("Carrier detected on channel ");
            Serial.print(ch, DEC);
            eol=true;
        } else {
            Serial.print(".");
            eol = (!((ch+1)%10) ? true : false);
        }

        chip_disable();
    }
    if (!eol) Serial.println();
}
