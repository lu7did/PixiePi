#!/bin/sh
#*-----------------------------------------------------------------------
#* Pi4D.sh
#* Script to implement a SSB transceiver
#* A remote pipe is implemented to carry CAT commands
#* Sound is feed thru the arecord command (PulseAudio ALSA), proper hardware
#* interface needs to be established. (-a parameter enables VOX)
#* the script needs to be called with the frequency in Hz as a parameter
#*            ./Pi4D.sh [frequency in Hz]
#*-----------------------------------------------------------------------
echo "Pi4D PixiePi based SSB transceiver ($date)"
echo "Frequency defined: $1"
/home/pi/PixiePi/bash/LCDoff.py

socat -d -d pty,raw,echo=0,link=/tmp/ttyv0 TCP-LISTEN:8080 &
PID=$!
echo "Pipe for /tmp/ttyv0 PID($PID)"
arecord -c1 -r48000 -D hw:1 -fS16_LE - | sudo /home/pi/PixiePi/bin/Pi4D -i /dev/stdin -s 6000 -p /tmp/ttyv0 -f "$1" -a
echo "Removing /tmp/ttyv0 PI($PID)"
sudo pkill socat
/home/pi/PixiePi/bash/LCDoff.py

