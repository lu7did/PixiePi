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

sudo rm -r /tmp/ssbpipe
mkfifo /tmp/ssbpipe || exit
#arecord -c1 -r48000 -D hw:1 -fS16_LE - | csdr convert_i16_f \
#  | csdr fir_interpolate_cc 2 | csdr dsb_fc \
#  | csdr bandpass_fir_fft_cc 0.002 0.06 0.01 | csdr fastagc_ff > /tmp/ssbpipe &
arecord -c1 -r48000 -D hw:CARD=Device,DEV=0 -fS16_LE - |
    csdr convert_i16_f |
    csdr dsb_fc |
    csdr bandpass_fir_fft_cc 0 0.1 0.01 |
    csdr gain_ff 2.0 |
    csdr shift_addition_cc 0.2 >  /tmp/ssbpipe &

SSB=$!


sudo /home/pi/PixiePi/bin/pixie -g 4 -f 7032000 -F 700 -G 20 -s /tmp/ttyv1 -i /home/pi/PixiePi/src/pixie/pixie.cfg


echo "Killing rigtcl PID($RIGCTL)"
sudo pkill rigctld
echo "Killing socat PID($PID)"
sudo pkill socat

echo "Killing SSB pipeline ($SSB)"
rm -f /tmp/ssbpipe  2>&1 > /dev/null

