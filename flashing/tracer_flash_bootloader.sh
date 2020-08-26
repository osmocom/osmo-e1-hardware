#!/bin/sh

cat e1tracer-bootloader-20200822.bin 1M0xff.bin > tmp
dd if=tmp of=flashme1M.bin bs=1024 count=1024
rm tmp
flashrom -p serprog:dev=/dev/ttyACM3:4000000 -w flashme1M.bin
