PixiePi
=======

Ham radio Pixie Transceiver controlled by a Raspberry Pi Zero W board


Requirements
   sudo apt-get install i2c-tools libi2c-dev
   git clone git://git.drogon.net/wiringPi

Build

cd /home/pi/PixiePi/src
make
sudo make install
 

Release notes:

  * This project requires a board combining:
     - Raspberry Pi
     - Rotary encoder
     - LCD 16x2 display
     - A Pixie transceiver (or similar)
  * Work in progress, this code set is not yet functional
