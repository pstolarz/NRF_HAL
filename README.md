NRF HAL port for Arduino
========================

This is a port of Nordic Semiconductor nRF24L01 transceiver's Hardware
Abstraction Layer API (NRF HAL API) for the Arduino platform. All the original
HAL function prototypes have been preserved plus some additions were introduced
(mostly getters for existing setters).

The HAL API provides an easy and flexible way to reach for the full power of
the nRF24 transceiver, abstracting an access for the device's low level registers,
communication  protocol etc. The API set consists of about 70 rather simple
functions performing basic actions on the transceiver (like opening an RX pipe,
setting TX/RX addresses, setting TX output power, checking various statuses,
reading/writing payload etc.)

See `./src/nrf_hal.h` header file for the API specification and `./examples` for
some examples.

Looking for NRF HAL API port for Raspberry PI, check out `librasp` library:

https://github.com/pstolarz/librasp

License
-------

Nordic Semiconductor Standard Software Development Kit License Agreement
