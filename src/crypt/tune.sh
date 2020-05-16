
#!/bin/sh
#*-----------------------------------------------------------------*
#* Pi4D.sh
#* Experimental SSB generator chain for FT8 transceiver
#*
#* Pi4D.sh  FREQ [in Hz]
#* Copyright 2019 (c) LU7DID 2019
#*-----------------------------------------------------------------*
#  | csdr fir_interpolate_cc 2 \
#  | csdr fractional_decimator_ff 10.0 --prefilter \

gpio mode "12" out
gpio -g write "12" 1
sudo /home/pi/rpitx/tune -f 7074000
gpio -g write "12" 0

