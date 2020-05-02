#!/bin/sh
#*-----------------------------------------------------------------------
#* PixiePi
#* Script to implement a simple USB transceiver using the OrangeThunder platformm
#* A remote pipe is implemented to carry CAT commands
#* Sound is feed thru the arecord command (PulseAudio ALSA), proper hardware
#* interface needs to be established. (-v parameter enables VOX)
#* the script needs to be called with the frequency in Hz as a parameter
#*            ./demo_genSSB.sh   (transmit at 14074000 Hz)
#*-----------------------------------------------------------------------
clear
echo "$0 (PixiePi simple USB transceiver)"

#*----------------------------------------*
#* Launching socat server                 *
#*----------------------------------------*
sudo socat -d -d pty,raw,echo=0,mode=0777,link=/tmp/ttyv0 pty,raw,echo=0,mode=0777,link=/tmp/ttyv1 2> /dev/null  &
PID=$!
echo "CAT commands piped from apps thru /tmp/ttyv1 PID($PID)"
#*----------------------------------------*
#* Launching rigctl server (optional)     *
#*----------------------------------------*
#echo "Launchiing rigctld"
#sudo rigctld -m 351 -r /dev/ttyS10 -T 127.0.0.1 -t 4532 -s 4800 --model=$(rigctl -l | grep "FT-817" | awk '{print $1}') -v &
#RIGCTL=$!
#echo "rigctld PID($RIGCTL)"

#*----------------------------------------*
#* Process clean up                       *
#*----------------------------------------*
sudo rm -r /tmp/ptt_fifo 2> /dev/null

sudo pkill -9 -f sendiq  2> /dev/null
sudo pkill -9 -f wsjtx   2> /dev/null
sudo pkill -9 -f flrig   2> /dev/null
sudo pkill -9 -f arecord 2> /dev/null

#*----------------------------------------*
#* Transceiver execution using loopback   *
#*----------------------------------------*
sudo Pi4D -p /tmp/ttyv0 -x  -f 7074000 

#*----------------------------------------*
#* terminating                            *
#*----------------------------------------*
sudo rm -r /tmp/ptt_fifo 2> /dev/null

echo "Killing rigtcl PID($RIGCTL)"
sudo pkill rigctld 2> /dev/null

echo "Removing /tmp/ttyv0 PI($PID)"
sudo pkill socat 2> /dev/null

echo "Removing /tmp/ptt_fifo"
sudo rm /tmp/ptt_fifo 2> /dev/null
echo "$0 completed"
#*----------------------------------------*
#*           End of Script                * 
#*----------------------------------------*

