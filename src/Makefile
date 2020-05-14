all: ../bin/pixie ../bin/PiWSPR ../bin/PiRTTY ../bin/Pi4D  ../bin/PixiePi

OSC_CFLAGS=-DOSCILLATOR_Z -DOSCILLATOR_D
CCP = c++
CC = cc
GCC = gcc

OT = /home/pi/OrangeThunder/src

CXXFLAGS = -std=c++11 -Wall -g -O3 -Wno-unused-variable
LDFLAGS = /home/pi/librpitx/src/librpitx.a -lm -lrt -lpthread -lwiringPi -lwiringPiDev -lpigpio 
CFLAGS  = -Wall -g -O3 -Wno-unused-variable
CXXFLAGS = -std=c++11 -Wall -g -O3 -Wno-unused-variable


../bin/PixiePi : PixiePi/PixiePi.cpp /home/pi/librpitx/src/librpitx.a lib/GUI.h lib/LCDLib.h lib/MMS.h $(OT)/lib/gpioWrapper.h $(OT)/lib/genVFO.h $(OT)/lib/CAT817.h lib/ClassMenu.h pixie/pixie.h lib/gui.h lib/DDS.h lib/iambic.c lib/VFO.h
	$(CCP) $(OSC_CFLAGS) $(CXXFLAGS) -o ../bin/PixiePi ./PixiePi/PixiePi.cpp ./minIni/minIni.c  $(LDFLAGS)

#../bin/pixie : pixie/pixie.cpp /home/pi/librpitx/src/librpitx.a lib/LCDLib.h $(OT)/lib/VFOSystem.h $(OT)/lib/CAT817.h lib/ClassMenu.h pixie/pixie.h lib/gui.h lib/DDS.h iambic/iambic.c minIni/minIni.c  minIni/minIni.h log.c/log.c iambic/iambic.c log.c/log.h 
#	$(CCP) $(OSC_CFLAGS) $(CXXFLAGS) -o ../bin/pixie ./pixie/pixie.cpp ./minIni/minIni.c ./log.c/log.c $(LDFLAGS)

../bin/Pi4D : Pi4D/Pi4D.cpp /home/pi/librpitx/src/librpitx.a minIni/minIni.c /home/pi/OrangeThunder/src/OT4D/OT4D.h
	$(CCP) $(OSC_CFLAGS) $(CXXFLAGS) -o ../bin/Pi4D ./Pi4D/Pi4D.cpp minIni/minIni.c  $(LDFLAGS)

../bin/PiWSPR : PiWSPR/PiWSPR.cpp /home/pi/librpitx/src/librpitx.a lib/DDS.h ./PiWSPR/PiWSPR.h lib/WSPR.h
	$(CCP) $(CXXFLAGS) -o ../bin/PiWSPR ./PiWSPR/PiWSPR.cpp  ./PiWSPR/PiWSPR.h lib/WSPR.h $(LDFLAGS)
 
../bin/PiRTTY : PiRTTY/PiRTTY.cpp  /home/pi/librpitx/src/librpitx.a
	$(CCP) $(OSC_CFLAGS) $(CXXFLAGS) -o ../bin/PiRTTY PiRTTY/PiRTTY.cpp  $(LDFLAGS)


clean:
	rm -r  ../bin/pixie
	rm -r  ../bin/PixiePi
	rm -r  ../bin/PiWSPR
	rm -r  ../bin/PiRTTY
	rm -r  ../bin/Pi4D

install: all
	install -m 0755 ../bin/pixie  /usr/bin
	install -m 0755 ../bin/PixiePi /usr/bin
	install -m 0755 ../bin/PiWSPR /usr/bin
	install -m 0755 ../bin/PiRTTY /usr/bin
	install -m 0755 ../bin/Pi4D   /usr/bin
