/*

Example of programming GPIO from C or C++ using the WiringPi library
on a Raspberry Pi.

Will continuously toggle GPIO24 (physical pin 18) at a 500 millisecond
rate.

Jeff Tranter <<a href="mailto:jtranter@ics.com">jtranter@ics.com</a>>

*/

#include <wiringPi.h>

int main(void)
{
    // Red LED: Physical pin 18, BCM GPIO24, and WiringPi pin 5.
    const int led = 12;

    wiringPiSetup();

    pinMode(led, OUTPUT);

    while (1) {
        digitalWrite(led, HIGH);
        delay(500);
        digitalWrite(led, LOW);
        delay(500);
    }

    return 0;
}
