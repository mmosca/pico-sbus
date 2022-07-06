#!/bin/bash
OPENOCD_IFACE=interface/raspberrypi-swd.cfg
OPENOCD_TARGET=target/rp2040.cfg

PREFIX=$1


set -x

if [ $PREFIX = "" ]
then

  openocd -f $OPENOCD_IFACE -f $OPENOCD_TARGET -c "program pico-sbus.elf verify reset exit"

else

  openocd -f $OPENOCD_IFACE -f $OPENOCD_TARGET -c "program $PREFIX/pico-sbus.elf verify reset exit"
fi
