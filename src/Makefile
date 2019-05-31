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


../pixie : pixie/pixie.cpp /home/pi/librpitx/src/librpitx.a pixie/LCDLib.h pixie/VFOSystem.h pixie/ClassMenu.h pixie/pixie.h pixie/gui.h
	$(CCP) $(CXXFLAGS) -o ../pixie ./pixie/pixie.cpp  $(LIBRPITX)

../DDSPi : DDSPi/DDSPi.cpp /home/pi/librpitx/src/librpitx.a DDSPi/DDSPi.h DDSPi/CAT817.h DDSPi/DDS.h
	$(CCP) $(CXXFLAGS) -o ../DDSPi ./DDSPi/DDSPi.cpp  $(LIBRPITX)

    
clean:
	rm -r  ../pixie
	rm -r  ../DDSPi

install: all
	install -m 0755 ../pixie /usr/bin
	install -m 0755 ../DDSPi /usr/bin
