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
acting as a DDS, which is thus the technical core of the project. A carrier generation function has been modelled largely using the tune.cpp program
available at the package also by Evariste Courjaud F5OEO. Some modifications has been introduced to allow the carrier frequency to be varied during the 
operation.

The rest of the code deals mostly with the user interface and operating features, among others:

* Receive over an entire ham radio band.

* Dual VFO setup, split mode, controllable step.

* RIT operation.

* Frequency can be shift during transmission to implement CW.

* Sidetone generation.

* Transmission controlled by processor.

* Display LCD & Rotary Encoder.

* Built-in iambic keyer, with variable speed.

* Variable DDS drive level.

* Capable of operating most digital modes with very small modifications.

* Perhaps, some day, even to manage SSB using a firmware approach (PE1NNZ)

# Fair and educated warning

Raspberry Pi is a marvel.

Hamradio is the best thing ever invented.

¡So don't ruin either by connecting GPIO04 directly to an antenna!

You'll make life of others in your neighboor unsormountable, and even
could get your Raspberry Pi fried in the process.

Google "raspberry pi rpitx low pass filter" to get some good advice on what to put between your Raspberry and your antenna
Or go to https://www.dk0tu.de/blog/2016/05/28_Raspberry_Pi_Lowpass_Filters/ for very good and easy to implement ideas

Remember that most national regulations requires the armonics and other spurious outcome to be -30 dB below the fundamental.

# Schematics

![Alt Text](docs/PixiePi.jpg?raw=true "PixiePi Schematics")

Pin VOL+ of the Pixie board connects to pin 7 of the IC LM386 which is usually left open without connection as a way to feed
the sidetone.


# Other alternatives

Even if the Pixie schematic is used for the project the software could be used directly with other DIY or homebrew popular
designs, among others:

* [PY2OHH's Curumim](http://py2ohh.w2c.com.br/trx/curumim/curumim.htm)
* [NorCal 49'er](http://www.norcalqrp.org/files/49er.pdf)
* [Miss Mosquita](https://www.qrpproject.de/Media/pdf/Mosquita40Engl.pdf)
* [Mosquito](http://www.qrp.cat/ea3ghs/mosquito.pdf)
* [Jersey Fireball](http://www.njqrp.club/fireball40/rev_b/fb40b_manual.pdf)

Lots of good QRPp projects can be found at [link](http://www.ncqrpp.org/) or SPRAT magazine [link](http://www.gqrp.com/sprat.htm).

# Case 3D Design

The preliminar 3D design for a project case (with LCD) can be found here

![Alt Text](docs/PixiePi_con_LCD.stl?raw=true "PixiePi Case 3D Design")

Warning: This 3D design is at prototype level and still requires work to be finalized


# Package requirements

*   sudo apt-get install i2c-tools libi2c-dev
*   sudo apt-get install socat
*   git clone git://git.drogon.net/wiringPi
*   git clone https://github.com/F5OEO/librpitx && cd librpitx/src && make 
*   Enable I2C and SPI with sudo raspi-config
*   Follow instructions to download and buid the FLRIG package from [here](http://www.w1hkj.com/flrig-help/)

# Build

*  git clone https://github.com/lu7did/PixiePi
*  cd /home/pi/PixiePi/src
*  make
*  sudo make install
 

# Release notes:

  * This project requires a board combining:

     - Raspberry Pi
     - Pixie Transceiver
     - Glueware simple electronics to switch transmitter, connect keyer and others.

  This setup can be used with flrig as the front-end and CAT controller, it should work with any
  other software supporting a Yaesu FT-817 model CAT command set.

# Other programs

  * DDSPi
    - DDS function controllable thru CAT

  * OT817
    - Transceiver USB controllable thru CAT

  * PiWSPR
    - WSPR beacon using a QRPp transceiver

  * pirtty
    - RTTY beacon

  * iambic-keyer
    An iambic-keyer for code practice

  * Optional 
     - Rotary encoder
     - LCD 16x2 display
     - USB soundcard (optional)

# Other packages

In general the hardware can be used to implement modulation modes proposed by the [rpitx package](https://github.com/F5OEO/rpitx).
In some cases the RF chain after the Raspberry Pi needs to be activated, the hardware on this project uses the
GPIO19 line as the PTT, some programs might require this line to be activated or deactivated externally.

#  Work in progress, this code set is not yet functional
