#!/bin/sh
#csdr fractional_decimator_ff 2

(while true; do cat sampleaudio.wav; done)  | csdr convert_i16_f | csdr fractional_decimator_ff 4.0 --prefilter \
  | csdr fir_interpolate_cc 2 |  csdr dsb_fc \
  | csdr bandpass_fir_fft_cc 0.002 0.24 0.05 | csdr fastagc_ff \
  | sudo /home/pi/rpitx/sendiq -i /dev/stdin -s 24000 -f "$1" -t float

