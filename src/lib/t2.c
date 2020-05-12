/*

Example of programming GPIO from C or C++ using the WiringPi library
on a Raspberry Pi.

Will read a pushbutton switch on GPIO6 (physical pin 31) every 500
milliseconds and report the status. seconds and then exit.

Jeff Tranter <<a href="mailto:jtranter@ics.com">jtranter@ics.com</a>>

*/

#include <stdio.h>
#include <wiringPi.h>

int main(void)
{
    // Switch: Physical pin 31, BCM GPIO6, and WiringPi pin 22.
    const int button = 29;

    wiringPiSetup();

    pinMode(button, INPUT);

    while (1) {
        if (digitalRead(button) == LOW) {
            fprintf(stderr, "Switch is pressed\n");
        } else {
            fprintf(stderr, "Switch is released\n");
        }
        delay(500);
    }

    return 0;
}
