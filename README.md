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

* Transmission controlled by processor.

* Display LCD & Rotary Encoder.

* Built-in iambic keyer, with variable speed.

* Variable DDS drive level.

* Capable of operating other digital modes, specially FT8 and WSPR, with very small modifications.

* Full or Partial Break-In in transmit.

* Open source firmware allow the adaptation to other simple DIY QRP/QRPp projects.

* With some limitations able to be used in linear modes (USB).

## DISCLAIMER

This is a pure, non-for-profit, project being performed in the pure ham radio spirit of experimentation, learning and sharing.
This project is original in conception and has a significant amount of code developed by me, it does also being built up on top
of a giant effort of others to develop the base platform and libraries used.
Therefore this code should not be used outside the limits of the license provided and in particular for uses other than
ham radio or similar experimental purposes.
No fit for any purpose is claimed nor guaranteed, use it at your own risk. The elements used are very common and safe, the skills
required very basic and limited, but again, use it at your own risk.
Despite being a software enginering professional with access to technology, infrastructure and computing facilities of different sorts
 I declare this project has been performed on my own time and equipment.

## Fair and educated warning

Raspberry Pi is a marvel.

Hamradio is the best thing ever invented.

¡So don't ruin either by connecting GPIO04 directly to an antenna!

You'll make life of others in your neighboor unsormountable, and even
could get your Raspberry Pi fried in the process.

Google "raspberry pi rpitx low pass filter" to get some good advice on what to put between your Raspberry and your antenna
Or go to https://www.dk0tu.de/blog/2016/05/28_Raspberry_Pi_Lowpass_Filters/ for very good and easy to implement ideas

Remember that most national regulations requires the armonics and other spurious outcome to be -30 dB below the fundamental.

# Schematics

## Complete with 16x2 LCD and Rotary Encoder

![Alt Text](docs/PixiePi.jpg?raw=true "PixiePi Schematics with 16x2 LCD Display and Rotary Encoder")

## Headless operation (all controls thru CAT) 

Headless operation is feasible by not connecting the Encoder, AUX switch and LCD display, however the power supply and keyer
circuit are still required for proper operation.

When used in Headless mode the transceiver behaviour can still be controlled by using CAT commands
see FLRig  and RigCtl usage in the incoming sections.


## Raspberry Pi Zero Pinout

The Raspberry Pi Zero pinout assignments are:

![Alt Text](docs/RaspberryPiZero_Pinout_anotado.jpg?raw=true "Raspberry Pi Zero Pinout")

## Hardware prototype

This is a snapshot of the current prototype used to develop and debug this project:

![Alt Text](docs/PixiePi_buildprocess.jpg?raw=true "PixiePi Hardware Prototype during testing")
![Alt Text](docs/PixiePi_Build_001.jpeg?raw=true "PixiePi Hardware Prototype")
![Alt Text](docs/PixiePi_Build_001.jpeg?raw=true "PixiePi Hardware Prototype")



# Chinese Pixie MODS

A typical circuit for the kit might be:

![Alt Text](docs/PixiePi_Schematics.jpg?raw=true "PixiePi Schematics")

Some minor modifications are needed while building the Chinese DIY Pixie kit, other versions might vary.

## Components not needed

The following components needs not to be placed when building the kit

```
* D2 - Diode 1N4001
* R6 - R 100K
* C8 - C 100nF
* W1 - R 47K (var)
* D3 - Diode 1N4148
* Y1 - Cristal 7.032 MHz
```
## Different connections (Recommended MODS)

```
* Connect Cx=100 nF on the same place than Y1 on the kit.
* Connect negative side of D3 diode to the interface board PTT line
* Cut trace from R5 to KEY socket, connect both keyer legs to the socket going into GPIO15 and GPIO13.
* Assure all three boards (interface, Pixie and Raspberry Pi) share a common ground.
* Extract +12V from the Pixie +12V socket, feed LM7805 with it, then feed the Raspberry Pi Zero board with it.
```
![Alt Text](docs/pixie_pcb.jpg?raw=true "PixiePi PCB mods")

All additional interface circuitry might be constructed on a prototype perfboard or using the Manhattan
technique.

## Pixie Final PA heat sink

Operating with a 12V supply the final transistor might become quite hot and indeed fail if the keydown period is long enough
(as it might be with FT8 or WSPR, even for slow CW). A small heatsink is recommended.
Space is very limited on typical kits but a small piece of aluminum might be enough, be aware not to short either L1 nor L3
with it.

The keyer transistor Q1 will benefit from a heat sink as well if long keying times are expected.

A cooler can be activated using the GPIO 19 line, feed the cooler not from the Raspberry Pi board but from the +5V regulator or
external power supply if used.


## Broadcast Interference (BCI)

At some locations the Chinese DIY Pixie kit might be subject to heavy BCI, in order to minimize try to replace R3 from
1K to 47-100 Ohms.

## Increase power and other features

An interesting set of low cost modifications to increase the power, improve efficiency and other enhancements to the original DIY Kit
can be found at [link](http://vtenn.com/Blog/?p=1348).

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

The preliminar 3D design for a project case (with LCD) can be seen as follows


[PixiePi 3D Case Design](https://www.thingiverse.com/thing:4153649)

**Warning**

3D STL file is an unfinished work in progress (see pending at issues)


# Package requirements

```
*   sudo apt-get install i2c-tools libi2c-dev
*   sudo apt-get install socat
*   sudo apt-get install telnet
*   git clone git://git.drogon.net/wiringPi
*   git clone https://github.com/F5OEO/librpitx && cd librpitx/src && make 
*   Enable I2C and SPI with sudo raspi-config
*   Follow instructions to download and build the HamLib package from https://sourceforge.net/projects/hamlib/files/hamlib/  (hamlib-1.0.1.tgz at this moment, check it out for latest)
*   Follow instructions to download and buid the FLRIG package from [here](http://www.w1hkj.com/flrig-help/)
*   Enable PWM in your Raspberry Pi [tutorial](https://learn.adafruit.com/adding-basic-audio-ouput-to-raspberry-pi-zero/pi-zero-pwm-audio) 
```

# Build

```
*  git clone https://github.com/lu7did/PixiePi
*  cd /home/pi/PixiePi/src
*  make
*  sudo make install
``` 

# Release notes:

  * This project requires a hardware board combining:

```
     - Raspberry Pi Zero (W preferred).
     - Pixie Transceiver
     - Glueware simple electronics to switch transmitter, connect keyer and others.
```
  This setup can be used with flrig as the front-end and CAT controller (**headless mode**), it should work with any
  other software supporting a Yaesu FT-817 model CAT command set.

# Operating WSPR

WSPR can be operated either as a monitoring station or as a beacon.

## Monitoring station

* Plug the PHONE exit to the LINE-IN entry of a soundcard.
* Start the pixie program with ./bash/pixie.sh (correct frequency to 7074000).
* Use WSJTX to monitor, select Mode as WSPR
* Ctl-C to terminate.

Your mileage might vary depending on local CONDX, antenna available and overall noise floor at your location,
some fairly strong signals can be decoded this way, after all it's just a very simple (tiniest imaginable)
double conversion receiver at work. Follows an example of a report captured from a WSPR Beacon in PY-Land some
500+ miles North of me.

![Alt Text](docs/PixiePi_WSPR.jpg?raw=true "WSJT-X Program using PixiePi as the receiver for WSPR monitoring")


## Beacon station

* Run ./bash/PiWSPR.sh (replace your call, grid and power). A sample script can be found at bash/PiWSPR.sh
This program will run once, so call it repeatedely with a timing of your choice, WSPR frames align and fire on even minutes.

Follows a screen capture of some reports given by the WSPRNet monitoring network[link](http://www.wsprnet.org), a fair distance has been achieved considered
the power output were in the order of 200 mW during the tests!

![Alt Text](docs/PixiePi_Reports_WSPR.png?raw=true "Reports from WSPRNet during the test")


# Operating FT8

FT8 can be operated either as a monitoring station or as a beacon or as a stand-alone USB transceiver for digital modes (see operating as an
USB transceiver below).

## Monitoring station

Same as WSPR monitoring station but selecting 7074000 as the frequency and FT8 at WSJTX, your mileage might vary
depending on local CONDX, antenna settings and overall noise floor at your locations, it's just a very basic
transceiver so probably relatively strong signals will be detected.

Sample of FT8 receiving:

![Alt Text](docs/PixiePi_FT8.jpg?raw=true "WSJT-X Program using PixiePi as the receiver for FT8 monitoring")

## Beacon station

Run pift8 from the rpitx package, simultaneous monitoring and beaconing will require a larger Raspberry Pi in order
to accomodate the extra power to run WSJT-X. Sample script can be found at bash/PiFT8.sh

Follows a screen capture of some reports given by the PSKReporter monitoring network[link](http://pskreporter.info), a fair distance has been achieved considered
the power output were in the order of 200 mW during the tests!

![Alt Text](docs/PixiePi_Reports_FT8.png?raw=true "Reports from PSKReporter network during the test")


# Operating as a CW transceiver

Configure settings accordingly at the pixie.cfg file and fire bash/pixie.sh

The following settings can be configured using the CAT interface, most changes will be made permanent at the pixie.cfg
file upon termination.

## Keyer

Using the default configuration (KEYER_MODE=0) the keyer will work as a straight keyer, however the keying must be made
thru the interface in order for the program to detect changes and shift frequencies in CW accordingly. Other keyer modes
can be programed using (KEYER_MODE=1 and KEYER_MODE=2).
When programmed as modes other than CW or CWR the keyer will basically become a PTT line (low=ON,high=OFF).


# Operating as a SSB transceiver

SSB transmission, as usually understood, requires a linear transmitter and the Pixie implementation (or any similar project) behaves as a 
class-C mode, which means non-linear big time. If used to transmit voice, or any other mode where the amplitude of the signal carries
information, the result will be severely distorted and create many problems to nearby stations, let alone comply with technical 
specs all hams has to comply with.

However there are many modes which are created using a SSB signal, still the information is either PSK or FM modulated, as an example
digital modes (i.e. RTTY or PSK31), image modes (SSTV) or weak signal modes (WSPR, FT8, etc). On these modes even a non-linear PA
like the Pixie will do, as long as the generation of the signal itself is linear. And this is what the Pi4D program does.

This program works with the PixiePi setup as a "headless" configuration, so no LCD display nor encoder knob is handled, so the transceiver
is setup on start to the typical frequency. The PTT line doesn´t exists but the transmitter can be turn on either using a CAT interface or
a VOX system.

The Raspberry Pi Zero computer doesn´t have any sound input interface so an external USB card needs to be used, assuming the proper 
configuration is used the actual FT8 or PSK31 or SSTV signal can be feed and transmitted; the transceiver is set as a receiver while not
transmitting therefore it can feed another computer. In this way the PixiePi configuration can be used as a stand-alone transceiver.

Theoretically the WSJT-X program can be used on the very same board used to implement the PixiePi platform, however a Raspberry Pi Zero is
way too small to support both the taxing demand of creating the RF signal and the CPU intensive calculus needed to operate small signals.

For implementation see Pi4D.sh at the PixiePi/bash directory.

Some samples of reports using the WSJT-X program for both transmitting and receiving using the program Pi4D.

![Alt Text](docs/PixiePi_Test_20200503_RX.jpg?raw=true "WSJT-X Program using PixiePi as the receiver for FT8 operation")
![Alt Text](docs/PixiePi_Test_20200503_TX.jpg?raw=true "Reports from using WSJT-X Program with PixiePi as the transmitter for FT8 operation")


# Other programs

  * pirtty
    - RTTY beacon

  * Optional hardware 
     - Rotary encoder
     - LCD 16x2 display
     - USB soundcard (optional)

**Work in progress, this code set has limited functionality, hardware has many issues, build experience needed at this point.

# CAT Control

CAT control can be performed over the programs of this project as they implement a limited subset of the Yaesu FT-817 CAT Command set.
Commands are carried thru an internal pipe (created using the socat package, see bash/pixie.sh script for details) which looks to the
controlling program just like a serial port (usually /tmp/tty0).

Operated in "headless" mode, with no tunning control or LCD display, this feature can be used to operate the transceiver.

Two alternatives has been tested:

## FLRig
FLRig can be built on the Raspberry Pi and used to control the PixiePi rig just configuring it as a FT-817 with the serial port as
/tmp/ttyv0. The PixiePi program must be up a running and the pipe established when this program is run.
Being a graphic X-Window app FLRig is somewhat taxing on resources and might not perform well when using a Raspberry Pi Zero W, however
it will run just fine on a Raspberry Pi 3 or higher.

![Alt Text](docs/PixiePi_flrig.jpg?raw=true "Controlling a PixiePi instance using FLRIG")


## RigCtl

RigCtl is a companion program of the HamLib library, it's main advantage is  a console operation with a very low resources consumption,
therefore it can be used on a Raspberry Pi Zero W to control the rig. See bash/pixie.sh in order to understand the way to configure it.

The rigctl interface is a server running in the background (rigctld) which can be accessed using a telnet interface at localhost port 4532.

See the docs/rigctl_commands.txt for a summary of the commands or visit the Hamlib page for complete documentation, the script 
PixiePi/bash/pixie.sh has sample on how to implement a pipe based to enable rigctl.

# Other packages

In general the hardware can be used to implement modulation modes proposed by the [rpitx package](https://github.com/F5OEO/rpitx).
In some cases the RF chain after the Raspberry Pi needs to be activated, the hardware on this project uses the
GPIO12 line as the PTT, some programs might require this line to be activated or deactivated externally.

#  Work in progress, this code has limited functionality
