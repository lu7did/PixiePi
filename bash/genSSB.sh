#!/bin/sh

arecord -c1 -r48000 -D hw:1 -fS16_LE -   | sudo genSSB -d  -x -v 2  -t 2 | sudo sendRF -i /dev/stdin -d  -s 6000 -f 7074000
