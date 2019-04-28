import I2C_LCD_driver
from time import *
import time

mylcd= I2C_LCD_driver.lcd()
mylcd.lcd_display_string("Hello World!",1)

mylcd.backlight(0)
exit(0)

