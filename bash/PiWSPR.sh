#!/bin/sh
#*-----------------------------------------------------------------------
#* PiWSPR
#* Script to emit a WSPR beacon frame
#* Progra is to emit every 120 secs
#*-----------------------------------------------------------------------

while true;
   do
   sudo PiWSPR -f 40m -c LU7DID -l GF05 -d 7 $1
   echo "Sleeping for 100 secs"
   sleep 100 

done


