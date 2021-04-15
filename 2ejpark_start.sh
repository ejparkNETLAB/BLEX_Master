#!/bin/bash
rm -rf build/
west build -b nrf52840dk_nrf52840 samples/basic/blinky
west flash
rm -rf build/
west build -b nrf52840dk_nrf52840 samples/bluetooth/throughput
west flash


