#!/bin/sh
#*-----------------------------------------------------------------------
#* PiFT8
#* Using the program PiFT8 from F5OEO (Evariste Coujard) to transmit FT8 frames
#* with the PixiePi hardware
#*
#*-----------------------------------------------------------------------

while true;
   do
     gpio mode "12" out
     gpio -g write "12" 1
     sudo pift8 -f 7074000 -m "CQ LU7DID GF05" -o 1240 -s 0
     gpio -g write "12" 0
     echo "sleeping 40 secs"
     sleep 40
   done

