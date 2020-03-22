#!/bin/sh

gpio mode "12" out
gpio -g write "12" 1

cat sampleaudio.wav  | sudo /home/pi/PixiePi/bin/Generator -i /dev/stdin -s 12000 -f "$1"

gpio -g write "12" 0

