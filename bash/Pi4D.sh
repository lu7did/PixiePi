#!/bin/sh
#*-----------------------------------------------------------------------
#* Pi4D
#* Script to establish a remote pipe with socal and then bring pixie up
#* this way enables a conduit for CAT commands to be sent remotely to this
#* instance
#*-----------------------------------------------------------------------
#socat -d -d pty,raw,echo=0,link=/tmp/ttyv0 TCP-LISTEN:8080 &
#PID=$!
#echo "socat PID($PID)"

arecord -c1 -r48000 -D hw:1 -fS16_LE - | sudo /home/pi/PixiePi/bin/Pi4D -i /dev/stdin -s 6000 -a -p /tmp/ttyv0 -f "$1" $2 $3

#echo "Killing socat PID($PID)"
#sudo pkill socat

