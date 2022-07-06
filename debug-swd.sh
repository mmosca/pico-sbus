#!/bin/bash
OPENOCD_IFACE=interface/raspberrypi-swd.cfg
OPENOCD_TARGET=target/rp2040.cfg

set -x

openocd -f $OPENOCD_IFACE -f $OPENOCD_TARGET -f openocd-remote.cfg
