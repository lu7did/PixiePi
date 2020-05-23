#!/usr/bin/python3
import RPi.GPIO as GPIO
import time
 
#* Establish numbering system as  BCM
GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False) 
#* Configure GIPIO 27 as output
GPIO.setup(12, GPIO.OUT)
#* Turn off GPIO 27 
GPIO.output(12, GPIO.HIGH)
