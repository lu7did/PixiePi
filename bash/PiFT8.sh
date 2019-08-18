#!/bin/sh
#*-----------------------------------------------------------------------
#* pixie
#* Script to establish a local pipe with socal and then bring DDS up
#*
#*-----------------------------------------------------------------------
gpio mode "12" out
gpio -g write "12" 1
sudo /home/pi/rpitx/pift8 -f 7074000 -m "CQ LU7DID GF05" -o 1240 -s 0
gpio -g write "12" 0


