#!/bin/python
import serial
import sys
arduino_port = argument = sys.argv[1]
baud_rate = 115200
arduino = serial.Serial(arduino_port, baud_rate)
while True:
    serial_data = arduino.readline().decode().strip()
    if serial_data:
        print(serial_data)
