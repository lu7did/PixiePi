#!/usr/bin/python
import I2C_LCD_driver
import sys
from time import *
mylcd = I2C_LCD_driver.lcd()

mylcd.backlight(1)
if len(sys.argv) == 1:
   exit()
if len(sys.argv) == 2:
   mylcd.lcd_display_string(sys.argv[1], 1)
   exit()
