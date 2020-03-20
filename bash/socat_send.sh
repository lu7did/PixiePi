#!/bin/sh
socat -d -d -d PTY,raw,echo=0,link=/tmp/ttyv0 TCP:ecrux:8080 2> /dev/null > /dev/null &
