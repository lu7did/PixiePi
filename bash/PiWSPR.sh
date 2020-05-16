#!/bin/sh
#*-----------------------------------------------------------------------
#* PiWSPR
#* Script to emit a WSPR beacon frame
#* Progra is to emit every 120 secs
#*-----------------------------------------------------------------------

while true;
   do
   sudo /home/pi/PixiePi/bin/PiWSPR -f 40m -c LU7DID -l GF05 -d 20 $1
   sleep 120 

done


