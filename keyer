#!/bin/sh
#//*========================================================================
#// keyer
#//
#// keyer start|stop|restart
#//
#// Iambic keyer for the Raspberry Pi.
#// See accompanying README file for a description on how to use this code.
#// License:
#//   This program is free software: you can redistribute it and/or modify
#//   it under the terms of the GNU General Public License as published by
#//   the Free Software Foundation, either version 2 of the License, or
#//   (at your option) any later version.
#//
#//   This program is distributed in the hope that it will be useful,
#//   but WITHOUT ANY WARRANTY; without even the implied warranty of
#//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#//   GNU General Public License for more details.
#//
#//   You should have received a copy of the GNU General Public License
#//   along with this program.  If not, see <http://www.gnu.org/licenses/>.
#//*========================================================================
#// lu7did: initial load
#*----------------------------------------------------------------------------
#* Initialization
#* DO NOT RUN EITHER AS A rc.local script nor as a systemd controlled service
#*----------------------------------------------------------------------------
#* Environment setup
#-----------------------------------------------------------------------------
DPATH="/home/pi/iambic-keyer"
cd $DPATH

case "$1" in
start)   echo -n "keyer $(date): Start keyer services" 2>&1 | tee -a $DPATH/keyer.log
         PIDL=$(sudo pgrep iambic)
         N=0
         for line in $PIDL; do
           if [ $line -ne $$ ] 
           then
              N=$(( $N + 1 ))
           fi
         done

         if [ $N \> 0 ]; then
            echo "keyer $(date): keyer program running, exit(1)"  2>&1 | tee -a $DPATH/keyer.log
            exit 1
         fi

         export `dbus-launch | grep ADDRESS`
         export `dbus-launch | grep PID`
         trap 'echo Terminating...' INT

         #*-----------------------------------------------------------------------------
         #* Load jackd server as a background process
         #*-----------------------------------------------------------------------------
         jackd -P70 -p16 -t2000 -dalsa -dhw:1,0 -p512 -n4 -r48000 -s  &
         #PID=$!

         #*-----------------------------------------------------------------------------
         #* Load iambic
         #*-----------------------------------------------------------------------------
         /home/pi/iambic-keyer/iambic -s 20 -m 2 & 
         #*-----------------------------------------------------------------------------
         #* Terminate
         #*-----------------------------------------------------------------------------
         exit 0
         ;;
stop)   echo -n "Keyer $(date): Stop keyer services\n"  2>&1 | tee -a $DPATH/keyer.log
        echo "Keyer $(date): Stop jackd PID(" $(pgrep 'jackd') ") " $(sudo pkill 'jackd')  2>&1 | tee -a $DPATH/keyer.log
        ;;
restart)
        echo -n "Keyer $(date): Keyer restart"  2>&1 | tee -a $DPATH/keyer.log
        $0 stop
        $0 start
        ;;
*)   echo "Usage: $0 start|stop|restart"  2>&1 | tee -a $DPATH/keyer.log
        exit 1
        ;;
esac
exit 0
