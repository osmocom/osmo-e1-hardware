#!/bin/sh

dfu-util -d 1d50:6150 -c 1 -a 0 -D riscv_usb.bin
dfu-util -d 1d50:6150 -c 1 -a 1 -D fw_app.bin -R
