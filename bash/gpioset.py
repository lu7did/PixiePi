#!/usr/bin/python3
#// gpioset [GPIO pin] [0|1]
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

#// lu7did: initial load
#*-- Import required libraries
import RPi.GPIO as GPIO
import time
import sys

port=int(sys.argv[1]);
st=int(sys.argv[2]);
print("gpioset: GPIO pin(%d) state(%d) " % (port,st));
 
#* Establish numbering convention, BCM in this case
GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)
#* Configure pin as output and set it
GPIO.setup(port, GPIO.OUT)
GPIO.output(port, st)


