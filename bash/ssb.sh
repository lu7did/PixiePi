#!/bin/sh
#*-----------------------------------------------------------------*
#* ssb.sh
#* Experimental SSB generator chain for FT8 transceiver
#*
#* ssb.sh FREQ [in Hz]
#*-----------------------------------------------------------------*
#arecord -c1 -r48000 -D hw:1 -fS16_LE - | csdr convert_i16_f \
#  | csdr fir_interpolate_cc 2 | csdr dsb_fc \
#  | csdr bandpass_fir_fft_cc 0.002 0.06 0.01 | csdr fastagc_ff \
#  | sudo /home/pi/rpitx/sendiq -i /dev/stdin -s 96000 -f "$1" -t float

arecord -c1 -r48000 -D hw:CARD=Device,DEV=0 -fS16_LE - |
    csdr convert_i16_f |
    csdr dsb_fc |
    csdr bandpass_fir_fft_cc 0 0.1 0.01 |
    csdr gain_ff 2.0 |
    csdr shift_addition_cc 0.2 |
    sudo /home/pi/rpitx/rpitx -i- -m IQFLOAT -f 7180
