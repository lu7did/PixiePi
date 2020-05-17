#!/bin/sh
#*-----------------------------------------------------------------------
#* PiWSPR
#* Script to emit a WSPR beacon frame
#* Progra is to emit every 120 secs
#*-----------------------------------------------------------------------

while true;
   do
   python gpioset.py 12 1
   python gpioset.py 24 1
   sudo PiWSPR -f 40m -c LU7DID -l GF05 -d 7 $1
   echo "Sleeping for 100 secs"
   python gpioset.py 12 0
   python gpioset.py 24 0
   sleep 100 

done


