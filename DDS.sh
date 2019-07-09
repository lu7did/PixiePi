#!/bin/sh
#*-----------------------------------------------------------------------
#* DDS
#* Script to establish a local pipe with socal and then bring DDS up
#*
#*-----------------------------------------------------------------------
socat -d -d pty,raw,echo=0,link=/tmp/ttyv0 pty,raw,echo=0,link=/tmp/ttyv1 &
PID=$!
echo "socat PID($PID)"

sudo /home/pi/PixiePi/DDSPi -g 4  -f 7032000 -s /tmp/ttyv1


echo "Killing socat PID($PID)"
sudo pkill socat



