#!/bin/bash
west build -b nrf52840dk_nrf52840 samples/bluetooth/throughputrealupdate
sleep 5s
west flash
