#!/bin/sh
#*-----------------------------------------------------------------------
#* pixie
#* Script to establish a local pipe with socal and then bring DDS up
#*
#*-----------------------------------------------------------------------
socat -d -d pty,raw,echo=0,link=/tmp/ttyv0 pty,raw,echo=0,link=/tmp/ttyv1 &
PID=$!
echo "socat PID($PID)"

rigctld --model=$(rigctl -l | grep "FT-817" | awk '{print $1}') -r /tmp/ttyv0 -t 4532 -s 4800 --set-conf=data_bits=8, stop_bits=2, serial_parity=None, serial_handshake=None, dtr_state=OFF, rts_state=OFF &
RIGCTL=$!
echo "rigctld PID($RIGCTL)"

sudo /home/pi/PixiePi/bin/pixie -g 4 -f 7074000 -F 700 -G 20 -s /tmp/ttyv1 -i /home/pi/PixiePi/src/pixie/pixie.cfg

echo "Killing rigtcl PID($RIGCTL)"
sudo pkill rigctld
echo "Killing socat PID($PID)"
sudo pkill socat

