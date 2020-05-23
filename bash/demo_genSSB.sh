#!/bin/sh
#*-----------------------------------------------------------------------
#* demo_genSSB
#* Script to implement a SSB transmitter using the OrangeThunder platformm
#* A remote pipe is implemented to carry CAT commands
#* Sound is feed thru the arecord command (PulseAudio ALSA), proper hardware
#* interface needs to be established. (-v parameter enables VOX)
#* the script needs to be called with the frequency in Hz as a parameter
#*            ./demo_genSSB.sh   (transmit at 14074000 Hz)
#*-----------------------------------------------------------------------
clear
#*----------------------------------------*
#* Launching socat server                 *
#*----------------------------------------*
sudo socat -d -d pty,raw,echo=0,mode=0777,link=/tmp/ttyv0 pty,raw,echo=0,mode=0777,link=/tmp/ttyv1 &
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
sudo rm -r /tmp/ptt_fifo

sudo pkill -9 -f sendiq
sudo pkill -9 -f wsjtx
sudo pkill -9 -f flrig
sudo pkill -9 -f arecord

#*----------------------------------------*
#* Transceiver execution using loopback   *
#*----------------------------------------*
sudo /home/pi/OrangeThunder/bin/demo_genSSB -i /dev/stdin -s 6000 -p /tmp/ttyv0 -f "$1" -t 3
#*----------------------------------------*
#* terminating                            *
#*----------------------------------------*
sudo rm -r /tmp/ptt_fifo

echo "Killing rigtcl PID($RIGCTL)"
sudo pkill rigctld

echo "Removing /tmp/ttyv0 PI($PID)"
sudo pkill socat

echo "Removing /tmp/ptt_fifo"
sudo rm /tmp/ptt_fifo
#*----------------------------------------*
#*           End of Script                * 
#*----------------------------------------*

