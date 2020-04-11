#!/usr/bin/python3
#// turnon
#// turn on bit at GPIO 27
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
 
#* Establish numbering convention, BCM in this case
GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)
#* Configure GPIO27 as output and turn it on
GPIO.setup(12, GPIO.OUT)
GPIO.output(12, GPIO.HIGH)
