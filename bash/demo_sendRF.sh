#!/bin/sh

arecord -c1 -r48000 -D hw:1 -fS16_LE - | genSSB | sudo /home/pi/OrangeThunder/bin/sendRF -i /dev/stdin -s 6000 -f 7074000 
