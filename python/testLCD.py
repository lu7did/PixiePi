import I2C_LCD_driver
from time import *
import time

mylcd= I2C_LCD_driver.lcd()
mylcd.lcd_display_string("Hello World!",1)

mylcd.backlight(0)

time.sleep(1)

mylcd.backlight(1)

mylcd.lcd_display_string("Hello World!", 2)

mylcd.lcd_display_string("This is how you", 1)
time.sleep(1)

mylcd.lcd_clear()

while True:
    mylcd.lcd_display_string("Hello world!",1)
    time.sleep(1)
    mylcd.lcd_clear()
    time.sleep(1)

while True:
    mylcd.lcd_display_string("Time: %s" %time.strftime("%H:%M:%S"), 1)
    mylcd.lcd_display_string("Date: %s" %time.strftime("%m/%d/%Y"), 2)


def get_ip_address(ifname):
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    return socket.inet_ntoa(fcntl.ioctl(
        s.fileno(),
        0x8915, 
        struct.pack('256s', ifname[:15])
    )[20:24])

mylcd.lcd_display_string("IP Address:", 1) 
mylcd.lcd_display_string(get_ip_address('eth0'), 2)
    

tr_pad = " " * 16
my_long_string = "This is a string that needs to scroll"
my_long_string = str_pad + my_long_string

while True:
    for i in range (0, len(my_long_string)):
        lcd_text = my_long_string[i:(i+16)]
        mylcd.lcd_display_string(lcd_text,1)
        sleep(0.4)
        mylcd.lcd_display_string(str_pad,1)
        


padding = " " * 16
my_long_string = "This is a string that needs to scroll"
padded_string = my_long_string + padding

for i in range (0, len(my_long_string)):
 lcd_text = padded_string[((len(my_long_string)-1)-i):-i]
 mylcd.lcd_display_string(lcd_text,1)
 sleep(0.4)
 mylcd.lcd_display_string(padding[(15+i):i], 1)
 ontdata1 = [      
        [ 0b00010, 
          0b00100, 
          0b01000, 
          0b10000, 
          0b01000, 
          0b00100, 
          0b00010, 
          0b00000 ],
]

mylcd.lcd_load_custom_chars(fontdata1)
mylcd.lcd_write(0x80)
mylcd.lcd_write_char(0)

