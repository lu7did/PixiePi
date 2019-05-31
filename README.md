# PixiePi

## Ham radio Pixie Transceiver controlled by a Raspberry Pi Zero W board

The Pixie QRPp (very low power, less than 1W output) transceiver is a very popular DIY project among hams as it is
very easy to build, test and operate from the electronic standpoint, yet able to perform some actual limited communications
(QSO) over the air.

Although really small power can be enough to sustain communications over great distances and therefore not being a per se
limiting factor there are other factors in the basic implementation which makes difficult to carry communications except
on very favorable conditions.

An explanation of how the transceiver work can be found [here](http://w1sye.org/wp-content/uploads/2017/01/NCRC_PixieOperation.pdf).

This project starts with a working Pixie transceiver (a cheap kit bought at eBay or other sellers) and to integrate it with
a Raspberry Pi to provide the signal generation and other functionality.

Instead of a crystal based signal generation the Raspberry Pi itself can be used by meas of the librpitx library (by Evariste Courjaud F5OEO
acting as a DDS, which is thus the technical core of the project. A carrier generation function has been extracted largely form the tune.cpp program
available at the package also by Evariste Courjaud F5OEO. Some modifications has been introduced to allow the carrier frequency to be varied during the 
operation.

The rest of the code deals mostly with the user interface and operating features, among others:

* Receive over an entire ham radio band.

* Dual VFO setup, split mode.

* Frequency can be shift during transmission.

* Sidetone generation.

* Transmission controlled by processor.

* Display LCD & Rotary Encoder.

* Built-in iambic keyer.

# Schematics

  Not yet available

# Requirements

   sudo apt-get install i2c-tools libi2c-dev
   sudo apt-get install socat

   git clone git://git.drogon.net/wiringPi

   git clone https://github.com/F5OEO/librpitx && cd librpitx/src && make 


# Build

   git clone https://github.com/lu7did/PixiePi

   cd /home/pi/PixiePi/src

   make

   sudo make install
 

# Release notes:

  * This project requires a board combining:

     - Raspberry Pi
     - Rotary encoder
     - LCD 16x2 display
     - A Pixie transceiver (or similar)
     - USB soundcard (optional)
 
 * Work in progress, this code set is not yet functional
