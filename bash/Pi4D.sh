
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
tinycap -- -D 1 -d 0 -c 1  | csdr convert_i16_f \
  | csdr dsb_fc \
  | csdr bandpass_fir_fft_cc 0.002 0.06 0.01 \
  | csdr fastagc_ff \
  | sudo /home/pi/rpitx/sendiq -i /dev/stdin -s 96000 -f "$1" -t float
gpio -g write "12" 0

