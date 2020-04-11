#!/usr/bin/python3
#// turnoff.py
#// utility to turn off GPIO27
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

#*-- Import GPIO and time libraries
import RPi.GPIO as GPIO
import time
 
#* Establish numbering system as  BCM
GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False) 
#* Configure GIPIO 27 as output
GPIO.setup(12, GPIO.OUT)
#* Turn off GPIO 27 
GPIO.output(12, GPIO.LOW)
