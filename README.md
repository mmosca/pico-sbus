# pico-sbus
Raspberry PI2040 SBUS library

Inspired by https://github.com/fdivitto/sbus and https://github.com/BrushlessPower/SBUS2-Telemetry

We use UART1 and GPIO5 as RX pin.

You will need both a logic level converter to change the RX voltage to 3.3v and a logic inverter.

A Sparkfun logic level converter should work: https://www.sparkfun.com/products/12009

Signal coming from the receiver needs needs to be inverted.
You can use a 7400 inverter like CD74HCT14E or a transistor based NOT gate as described here https://www.electronics-tutorials.ws/logic/logic_4.html


