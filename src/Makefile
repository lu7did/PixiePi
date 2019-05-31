all: ../pixie ../DDSPi 

OSC_CFLAGS=-DOSCILLATOR_Z -DOSCILLATOR_D
CXXFLAGS = -std=c++11 -Wall -g -O2 -Wno-unused-variable
LDFLAGS = -lm -lrt -lpthread -lwiringPi -lwiringPiDev -lpigpio  -lpthread
GCC = gcc
CFLAGS  = -Wall -g -O2 -Wno-unused-variable
CXXFLAGS = -std=c++11 -Wall -g -O2 -Wno-unused-variable
LIBRPITX = /home/pi/librpitx/src/librpitx.a -lm -lrt -lpthread -lwiringPi -lwiringPiDev -lpigpio 
CCP = c++
CC = cc


../pixie : pixie/pixie.cpp /home/pi/librpitx/src/librpitx.a lib/LCDLib.h lib/VFOSystem.h lib/ClassMenu.h pixie/pixie.h lib/gui.h
	$(CCP) $(CXXFLAGS) -o ../pixie ./pixie/pixie.cpp  $(LIBRPITX)

../DDSPi : DDSPi/DDSPi.cpp /home/pi/librpitx/src/librpitx.a DDSPi/DDSPi.h lib/CAT817.h lib/DDS.h
	$(CCP) $(CXXFLAGS) -o ../DDSPi ./DDSPi/DDSPi.cpp  $(LIBRPITX)

    
clean:
	rm -r  ../pixie
	rm -r  ../DDSPi

install: all
	install -m 0755 ../pixie /usr/bin
	install -m 0755 ../DDSPi /usr/bin
