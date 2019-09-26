#!/bin/sh


#-------------------------------------------------------------------------------------------------------------------------------
# fire rigctl -l to list devices looking for the id of the FT-817 model
# as an alternative fire telnet localhost 4532 to access the CAT interface
#-------------------------------------------------------------------------------------------------------------------------------
#rigctld --model=120 -r /tmp/ttyv0 -t 4532 -s 4800 --set-conf=data_bits=8, stop_bits=2, serial_parity=None, serial_handshake=None, dtr_state=OFF, rts_state=OFF

rigctl -m $(rigctl -l | grep "FT-817" | awk '{print $1}')  -r /tmp/ttyv0
