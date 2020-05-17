#!/bin/sh
#*-----------------------------------------------------------------------
#* PiWSPR
#* Script to emit a WSPR beacon frame
#* Progra is to emit every 120 secs
#*-----------------------------------------------------------------------
LAP="400"
CALLSIGN="LU7DID"
GRID="GF05"
POWER="7"
BAND="40m"
while true;
   do
   sudo PiWSPR -f $BAND -c $CALLSIGN -l $GRID -d $POWER $1
   echo "Sleeping for $LAP secs"
   sleep $LAP 

done


