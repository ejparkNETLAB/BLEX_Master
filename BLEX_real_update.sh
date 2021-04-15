#!/bin/bash
rm -rf build/
west build -b nrf52840dk_nrf52840 samples/basic/blinky
west flash
sleep 40s
rm -rf build/
west build -b nrf52840dk_nrf52840 samples/bluetooth/throughputrealupdate
west flash
