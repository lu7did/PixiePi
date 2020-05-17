#!/bin/sh
#*-----------------------------------------------------------------------
#* PixiePi
#* Script to launch the firmware for the PixiePi transceiver
#*
#* Raspberry Pi based CW HF transceiver controller
#*---------------------------------------------------------------------
#* This program operates as a controller for a Raspberry Pi to control
#* a Pixie transceiver hardware.
#* Project at http://www.github.com/lu7did/PixiePi
#*---------------------------------------------------------------------
#*-----------------------------------------------------------------------
#* Brings a local pipe to carry remote commands thru CAT (FT817)
#*-----------------------------------------------------------------------
socat -d -d pty,raw,echo=0,link=/tmp/ttyv0 pty,raw,echo=0,link=/tmp/ttyv1 &
PID=$!
echo "socat PID($PID)"
#*-----------------------------------------------------------------------
#* Enable for a local rigctl server
#*-----------------------------------------------------------------------
#rigctld --model=$(rigctl -l | grep "FT-817" | awk '{print $1}') -r /tmp/ttyv0 -t 4532 -s 4800 --set-conf=data_bits=8, stop_bits=2, serial_parity=None, serial_handshake=None, dtr_state=OFF, rts_state=OFF &
#RIGCTL=$!
#echo "rigctld PID($RIGCTL)"
#*------------------------------------------------------------------------
#* Launch program
#*------------------------------------------------------------------------
sudo PixiePi -f 7030000 -s 15 -m 2 -l 3 -p /tmp/ttyv1 -k 0 -c -x 600
#*------------------------------------------------------------------------
#* Terminate and cleau up resorces
#*------------------------------------------------------------------------
#echo "Killing rigtcl PID($RIGCTL)"
#sudo pkill rigctld
echo "Killing socat PID($PID)"
sudo kill -9 $PID
#*---- End
