#!/bin/sh
#*-----------------------------------------------------------------------
#* Pi4D
#* Script to implement a simple USB transceiver using the OrangeThunder platformm
#* A remote pipe is implemented to carry CAT commands
#* Sound is feed thru the arecord command (PulseAudio ALSA), proper hardware
#* interface needs to be established. (-v parameter enables VOX)
#* the script needs to be called with the frequency in Hz as a parameter
#*            ./demo_genSSB.sh   (transmit at 14074000 Hz)
#*-----------------------------------------------------------------------
clear
echo "$(date) $0 PixiePi simple USB transceiver"

#* Process clean up                       *
#*----------------------------------------*
sudo rm -r /tmp/ptt_fifo 2> /dev/null

sudo pkill -9 -f sendiq  2> /dev/null
sudo pkill -9 -f wsjtx   2> /dev/null
sudo pkill -9 -f flrig   2> /dev/null
sudo pkill -9 -f arecord 2> /dev/null

sudo modprobe snd-aloop
#*----------------------------------------*
#* Transceiver execution using loopback   *
#*----------------------------------------*
gpio write 12 1
sudo Pi4D -t 2  -x  -f 7074000 
gpio write 12 0
#*----------------------------------------*
#* terminating                            *
#*----------------------------------------*
sudo rm -r /tmp/ptt_fifo 2> /dev/null

echo "$(date) $0 Killing rigtcl PID($RIGCTL)"
sudo pkill rigctld 2> /dev/null

echo "$(date) $0 Removing /tmp/ttyv0 PI($PID)"
sudo pkill socat 2> /dev/null

echo "$(date) $0 Removing /tmp/ptt_fifo"
sudo rm /tmp/ptt_fifo 2> /dev/null
echo "$(date) $0 completed"
#*----------------------------------------*
#*           End of Script                * 
#*----------------------------------------*

