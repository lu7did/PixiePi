all: ../testLCD ../testEncoder  ../testRotary

CXXFLAGS = -std=c++11 -Wall -g -O2 -Wno-unused-variable
LDFLAGS = -lm -lrt -lpthread -lwiringPi -lwiringPiDev
GCC = gcc


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

install: all
	install -m 0755 ../testLCD /usr/bin
	install -m 0755 ../testEncoder /usr/bin
	install -m 0755 ../testRotary /usr/bin
