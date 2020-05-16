#!/bin/sh
#*-----------------------------------------------------------------------
#* PixiePi
#* Script to launch the firmware for the PixiePi transceiver
#*
#*-----------------------------------------------------------------------
#* Brings a local pipe to carry remote commands thru CAT
#*-----------------------------------------------------------------------
socat -d -d pty,raw,echo=0,link=/tmp/ttyv0 pty,raw,echo=0,link=/tmp/ttyv1 &
PID=$!
echo "socat PID($PID)"

#rigctld --model=$(rigctl -l | grep "FT-817" | awk '{print $1}') -r /tmp/ttyv0 -t 4532 -s 4800 --set-conf=data_bits=8, stop_bits=2, serial_parity=None, serial_handshake=None, dtr_state=OFF, rts_state=OFF &
#RIGCTL=$!
#echo "rigctld PID($RIGCTL)"

#*------------------------------------------------------------------------
#* Launch program
#*------------------------------------------------------------------------
sudo PixiePi -f 7030000 -s 15 -m 2 -l 3 -p /tmp/ttyv1 -v 2 -k 0

#echo "Killing rigtcl PID($RIGCTL)"
#sudo pkill rigctld

echo "Killing socat PID($PID)"
#sudo pkill socat
sudo kill -9 $PID

