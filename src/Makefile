all: ../bin/pixie ../bin/PiWSPR ../bin/PiRTTY 

OSC_CFLAGS=-DOSCILLATOR_Z -DOSCILLATOR_D

CXXFLAGS = -std=c++11 -Wall -g -O2 -Wno-unused-variable
LDFLAGS = /home/pi/librpitx/src/librpitx.a -lm -lrt -lpthread -lwiringPi -lwiringPiDev -lpigpio
CFLAGS  = -Wall -g -O2 -Wno-unused-variable
CXXFLAGS = -std=c++11 -Wall -g -O2 -Wno-unused-variable

CCP = c++
CC = cc
GCC = gcc

../bin/pixie : pixie/pixie.cpp /home/pi/librpitx/src/librpitx.a lib/LCDLib.h lib/VFOSystem.h lib/CAT817.h lib/ClassMenu.h pixie/pixie.h lib/gui.h lib/DDS.h iambic/iambic.c minIni/minIni.c  minIni/minIni.h log.c/log.c log.c/log.h
	$(CCP) $(OSC_CFLAGS) $(CXXFLAGS) -o ../bin/pixie ./pixie/pixie.cpp ./minIni/minIni.c lib/CAT817.h ./log.c/log.c $(LDFLAGS)

../bin/PiWSPR : PiWSPR/PiWSPR.cpp /home/pi/librpitx/src/librpitx.a lib/DDS.h ./PiWSPR/PiWSPR.h lib/WSPR.h
	$(CCP) $(CXXFLAGS) -o ../bin/PiWSPR ./PiWSPR/PiWSPR.cpp  ./PiWSPR/PiWSPR.h lib/WSPR.h $(LDFLAGS)
 
../bin/PiRTTY : PiRTTY/PiRTTY.cpp  /home/pi/librpitx/src/librpitx.a
	$(CCP) $(OSC_CFLAGS) $(CXXFLAGS) -o ../bin/PiRTTY PiRTTY/PiRTTY.cpp  $(LDFLAGS)


clean:
	rm -r  ../bin/pixie
	rm -r  ../bin/PiWSPR
	rm -r  ../bin/PiRTTY

install: all
	install -m 0755 ../bin/pixie  /usr/bin
	install -m 0755 ../bin/PiWSPR /usr/bin
	install -m 0755 ../bin/PiRTTY /usr/bin
