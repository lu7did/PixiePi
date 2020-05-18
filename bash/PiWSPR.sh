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
DPATH="/home/pi/PixiePi/bash/"
LOG="PiWSPR.log"
echo "$(date) $0 Executing Call($CALLSIGN) Grid($GRID) POWER($POWER) BAND($BAND) LAP($LAP secs)" 2>&1 | tee -a $DPATH$LOG
while true;
   do
   sudo PiWSPR -f $BAND -c $CALLSIGN -l $GRID -d $POWER $1 2>&1 | tee -a $DPATH$LOG
   echo "$(date) $0 Sleeping for $LAP secs" 2>&1 | tee -a $DPATH$LOG
   sleep $LAP 

done


