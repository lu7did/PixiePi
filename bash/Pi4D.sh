#!/bin/sh

gpio mode "12" out
gpio -g write "12" 1

#cat sampleaudio.wav  | sudo /home/pi/PixiePi/bin/Pi4D -i /dev/stdin -s $2 -f "$1" $3
arecord -c1 -r48000 -D hw:1 -fS16_LE - | sudo /home/pi/PixiePi/bin/Pi4D -i /dev/stdin -s $2 -f "$1" $3

gpio -g write "12" 0

