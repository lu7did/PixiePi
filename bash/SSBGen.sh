#!/bin/sh

gpio mode "12" out
gpio -g write "12" 1

cat sampleaudio.wav  | sudo /home/pi/PixiePi/bin/SSBGen -i /dev/stdin -s $2 -f "$1" $3

gpio -g write "12" 0

