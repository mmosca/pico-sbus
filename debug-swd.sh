#!/bin/bash
OPENOCD_IFACE=/usr/share/openocd/scripts/interface/raspberrypi-swd.cfg
OPENOCD_TARGET=/usr/share/openocd/scripts/target/rp2040.cfg

set -x

openocd -f $OPENOCD_IFACE -f $OPENOCD_TARGET
