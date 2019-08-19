all: ../bin/pixie ../bin/DDSPi ../bin/OT817 ../bin/PiWSPR ../bin/pirtty  

OSC_CFLAGS=-DOSCILLATOR_Z -DOSCILLATOR_D
CXXFLAGS = -std=c++11 -Wall -g -O2 -Wno-unused-variable
LDFLAGS = /home/pi/librpitx/src/librpitx.a -lm -lrt -lpthread -lwiringPi -lwiringPiDev -lpigpio
GCC = gcc
CFLAGS  = -Wall -g -O2 -Wno-unused-variable
CXXFLAGS = -std=c++11 -Wall -g -O2 -Wno-unused-variable
LIBRPITX = /home/pi/librpitx/src/librpitx.a -lm -lrt -lpthread -lwiringPi -lwiringPiDev -lpigpio 
CCP = c++
CC = cc

../bin/PiWSPR : PiWSPR/PiWSPR.cpp /home/pi/librpitx/src/librpitx.a lib/DDS.h ./PiWSPR/PiWSPR.h lib/WSPR.h
	$(CCP) $(CXXFLAGS) -o ../bin/PiWSPR ./PiWSPR/PiWSPR.cpp  ./PiWSPR/PiWSPR.h lib/WSPR.h $(LDFLAGS)
 
../bin/pixie : pixie/pixie.cpp /home/pi/librpitx/src/librpitx.a lib/LCDLib.h lib/VFOSystem.h lib/ClassMenu.h pixie/pixie.h lib/gui.h lib/CAT817.h lib/DDS.h iambic/iambic.c minIni/minIni.c  minIni/minIni.h log.c/log.c log.c/log.h
	$(CCP) $(OSC_CFLAGS) -o ../bin/pixie ./pixie/pixie.cpp ./minIni/minIni.c  ./log.c/log.c $(LIBRPITX)

../bin/DDSPi : DDSPi/DDSPi.cpp /home/pi/librpitx/src/librpitx.a DDSPi/DDSPi.h lib/CAT817.h lib/DDS.h
	$(CCP) $(CXXFLAGS) -o ../bin/DDSPi ./DDSPi/DDSPi.cpp  $(LIBRPITX)

../bin/OT817 : OT817/OT817.cpp /home/pi/librpitx/src/librpitx.a OT817/OT817.h lib/CAT817.h lib/DDS.h
	$(CCP) $(CXXFLAGS) -o ../bin/OT817 ./OT817/OT817.cpp  $(LIBRPITX)

../bin/pirtty : pirtty/pirtty.cpp  /home/pi/librpitx/src/librpitx.a
	$(CCP) $(CXXFLAGS) -o ../bin/pirtty pirtty/pirtty.cpp  $(LDFLAGS)


    
clean:
	rm -r  ../bin/pixie
	rm -r  ../bin/DDSPi
	rm -r  ../bin/OT817
	rm -r  ../bin/PiWSPR
	rm -r  ../bin/pirtty

install: all
	install -m 0755 ../bin/pixie  /usr/bin
	install -m 0755 ../bin/DDSPi  /usr/bin
	install -m 0755 ../bin/OT817  /usr/bin
	install -m 0755 ../bin/PiWSPR /usr/bin
	install -m 0755 ../bin/pirtty /usr/bin
