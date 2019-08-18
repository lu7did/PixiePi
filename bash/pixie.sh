#!/bin/sh
#*-----------------------------------------------------------------------
#* pixie
#* Script to establish a local pipe with socal and then bring DDS up
#*
#*-----------------------------------------------------------------------
socat -d -d pty,raw,echo=0,link=/tmp/ttyv0 pty,raw,echo=0,link=/tmp/ttyv1 &
PID=$!
echo "socat PID($PID)"

sudo /home/pi/PixiePi/bin/pixie -g 4 -f 7032000 -F 700 -G 20 -s /tmp/ttyv1 -d -i /home/pi/PixiePi/src/pixie/pixie.cfg


echo "Killing socat PID($PID)"
sudo pkill socat



