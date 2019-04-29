all: ../testLCD ../testEncoder  ../testRotary ../tune 

OSC_CFLAGS=-DOSCILLATOR_Z -DOSCILLATOR_D
CXXFLAGS = -std=c++11 -Wall -g -O2 -Wno-unused-variable
LDFLAGS = -lm -lrt -lpthread -lwiringPi -lwiringPiDev -lpigpio  -lpthread
GCC = gcc
CFLAGS  = -Wall -g -O2 -Wno-unused-variable
CXXFLAGS = -std=c++11 -Wall -g -O2 -Wno-unused-variable
LIBRPITX = /home/pi/librpitx/src/librpitx.a -lm -lrt -lpthread
CCP = c++
CC = cc



../tune : softDDS/tune.cpp /home/pi/librpitx/src/librpitx.a
	$(CCP) $(CXXFLAGS) -o ../tune ./softDDS/tune.cpp  $(LIBRPITX)

../testLCD : testLCD/testLCD.c 
	$(GCC) -o ../testLCD ./testLCD/testLCD.c $(LDFLAGS)

../testRotary : testRotary/testRotary.c
	$(GCC) -o ../testRotary ./testRotary/testRotary.c $(LDFLAGS)

../testEncoder : testEncoder/testEncoder.c
	$(GCC) -o ../testEncoder ./testEncoder/testEncoder.c $(LDFLAGS)

    
clean:
	rm -f  ../testLCD 
	rm -r  ../testEncoder
	rm -r  ../testRotary
	rm -r  ../tune

install: all
	install -m 0755 ../testLCD /usr/bin
	install -m 0755 ../testEncoder /usr/bin
	install -m 0755 ../testRotary /usr/bin
	install -m 0755 ../tune /usr/bin
