#!/bin/sh
#*-----------------------------------------------------------------------
#* pixieTCP
#* Script to establish a remote pipe with socal and then bring DDS up
#* 
#*-----------------------------------------------------------------------
#socat -d -d pty,raw,echo=0,link=/tmp/ttyv0 pty,raw,echo=0,link=/tmp/ttyv1,tcp:192.168.0.215:8080 &
socat -d -d pty,raw,echo=0,link=/tmp/ttyv0,raw,echo=0 TCP4:192.168.0.215:8080 &
PID=$!
echo "socat PID($PID)"


