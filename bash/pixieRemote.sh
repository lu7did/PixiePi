#!/bin/sh
#*-----------------------------------------------------------------------
#* pixieRemote
#* Script to establish a remote pipe with socal and then bring pixie up
#* this way enables a conduit for CAT commands to be sent remotely to this
#* instance
#*-----------------------------------------------------------------------
socat -d -d pty,raw,echo=0,link=/tmp/ttyv0 TCP-LISTEN:8080 &
PID=$!
echo "socat PID($PID)"

#rigctld --model=$(rigctl -l | grep "FT-817" | awk '{print $1}') -r /tmp/ttyv0 -t 4532 -s 4800 --set-conf=data_bits=8, stop_bits=2, serial_parity=None, serial_handshake=None, dtr_state=OFF, rts_state=OFF &
#RIGCTL=$!
#echo "rigctld PID($RIGCTL)"

sudo /home/pi/PixiePi/bin/pixie -g 4 -f 7030000 -F 700 -G 20 -s /tmp/ttyv0 -i /home/pi/PixiePi/src/pixie/pixie.cfg

#echo "Killing rigtcl PID($RIGCTL)"
#sudo pkill rigctld

echo "Killing socat PID($PID)"
sudo pkill socat

