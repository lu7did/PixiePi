// I2C device address//*--------------------------------------------------------------------------------------------------
//* LCDLib LCD Management Class   (HEADER CLASS)
//*--------------------------------------------------------------------------------------------------
//* Este es el firmware del diseÃ±o de LCD para PixiePi
//* Solo para uso de radioaficionados, prohibido su utilizacion comercial
//* Copyright 2018 Dr. Pedro E. Colla (LU7DID)
//*--------------------------------------------------------------------------------------------------
#ifndef LCDLib_h
#define LCDLib_h


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <sched.h>
#include <time.h>
#include <sys/mman.h>
#include <pthread.h>
#include <signal.h>
#include <semaphore.h>
#include <pigpio.h>
#include <wiringPiI2C.h>
#include <iostream>
using namespace std;

//*--- Definition for VFO parameters and limits

#define I2C_ADDR    0x27 

//*--- LCD constants (16x2 I2C controlled LCD display assumed)

#define LCD_CHR     1    // Mode - Sending data
#define LCD_CMD     0    // Mode - Sending command
#define LINE1       0x80 // 1st line
#define LINE2       0xC0 // 2nd line

#define LCD_ON      0x08
#define LCD_OFF     0x00
#define LCD_CGRAM   64
#define ENABLE      0b00000100 // Enable bit


//*--- Special characters used in the GUI

byte TX[8] = {0B11111,0B10001,0B11011,0B11011,0B11011,0B11011,0B11111}; // Inverted T (Transmission Mode)
byte SP[8] = {31,17,23,17,29,17,31};    //Inverted S (Split)
byte S1[8] = {0B10000,0B10000,0B10000,0B10000,0B10000,0B10000,0B10000,0B10000}; // S1 Signal
byte S2[8] = {0B11000,0B11000,0B11000,0B11000,0B11000,0B11000,0B11000,0B11000}; // S2 Signal
byte S3[8] = {0B11100,0B11100,0B11100,0B11100,0B11100,0B11100,0B11100,0B11100}; // S3 Signal
byte S4[8] = {0B11110,0B11110,0B11110,0B11110,0B11110,0B11110,0B11110,0B11110}; // S4 Signal
byte S5[8] = {0B11111,0B11111,0B11111,0B11111,0B11111,0B11111,0B11111,0B11111}; // S5 Signal
byte NA[8] = {0B11111,0B11011,0B10101,0B10101,0B10101,0B10001,0B10101,0B00000}; // Inverted A
byte NB[8] = {0B11111,0B10001,0B10101,0B10011,0B10101,0B10101,0B10001,0B00000}; // Inverted B
byte NS[8] = {0B11111,0B11001,0B10111,0B10011,0B11101,0B11101,0B10011,0B00000}; // Inverted S


typedef unsigned char byte;
typedef bool boolean;
typedef void (*CALLBACK)();

int fd;

//*---------------------------------------------------------------------------------------------------
//* VFOSystem CLASS
//*---------------------------------------------------------------------------------------------------
class LCDLib
{
  public: 

      LCDLib(CALLBACK c);  
      void begin(byte c, byte r);
      void createChar(byte n,byte* custom);
      void setCursor(byte c, byte r);
      void print(string s);
      void println(byte c,byte r,char* s);
      void write(byte c);
      void Blink();
      void noBlink();
      void typeFloat(float f);
      void typeInt(int i);
      void clear();  
      void lcdLoc(int line);
      void typeChar(char val);
      void backlight(boolean v);
      void typeln(const char *s);
      void lcd_byte(int bits, int mode);
      void lcd_toggle_enable(int bits);

      int  LCD_LIGHT;
  private:

};

#endif
//*---------------------------------------------------------------------------------------------------
//* LCDLib CLASS Implementation
//*---------------------------------------------------------------------------------------------------
LCDLib::LCDLib(CALLBACK c)
{
  
}
//*---------------------------------------------------------------------------------------------------
//* LCD Clear 
//*---------------------------------------------------------------------------------------------------
void LCDLib::clear() {

  lcd_byte(0x01, LCD_CMD);
  lcd_byte(0x02, LCD_CMD);

}
//*---------------------------------------------------------------------------------------------------
//* LCD start operation
//*---------------------------------------------------------------------------------------------------
void LCDLib::begin(byte c, byte r) {

  fd=wiringPiI2CSetup(I2C_ADDR);

 // Initialise display

  lcd_byte(0x33, LCD_CMD); // Initialise
  lcd_byte(0x32, LCD_CMD); // Initialise
  lcd_byte(0x06, LCD_CMD); // Cursor move direction
  lcd_byte(0x0C, LCD_CMD); // 0x0F On, Blink Off
  lcd_byte(0x28, LCD_CMD); // Data length, number of lines, font size
  lcd_byte(0x01, LCD_CMD); // Clear display
  delayMicroseconds(500);
  
  LCD_LIGHT=LCD_ON;

  return;
}
//*---------------------------------------------------------------------------------------------------
//* LCD Create custom char (n=0..7 custom=8 byte array)
//*---------------------------------------------------------------------------------------------------
void LCDLib::createChar(byte n,byte* custom) {

  lcd_byte (LCD_CGRAM | (n<<3), LCD_CMD);      //# Start definine char charnum, line 0
  for (int i=0;i<8;i++) {
      lcd_byte (custom[i], LCD_CHR); //# first line solid line (11111b)
  }
  lcd_byte (1, LCD_CMD);       //# Clear screen. Ends char define


  return;
}
//*---------------------------------------------------------------------------------------------------
//*  LCD set cursor c(0..15) r(0..1)
//*---------------------------------------------------------------------------------------------------
void LCDLib::setCursor(byte c, byte r) {
  
  int l;
  if (r==0) {
     l=LINE1;   //# start writing at line 1
  } else {
     l=LINE2;   //# start writing at line 1
  }
  c=c&0xf;
  lcdLoc(l+c);
  return;
}
//*---------------------------------------------------------------------------------------------------
//* LCD print (print char*) 
//*---------------------------------------------------------------------------------------------------
void LCDLib::println(byte c,byte r,char* s) {
  char cstr[strlen(s)+8];
  strcpy(cstr,s);
  this->setCursor(c,r);
  typeln(cstr);
}
//*---------------------------------------------------------------------------------------------------
//* LCD print (print string) 
//*---------------------------------------------------------------------------------------------------
void LCDLib::print(string s) {
  
  
  char cstr[s.size() + 1];
  strcpy(cstr, s.c_str());	// or pass &s[0]
  typeln(cstr);

  return;
}
//*---------------------------------------------------------------------------------------------------
//* LCD write custom character N (0..7)
//*---------------------------------------------------------------------------------------------------
void LCDLib::write(byte c) {
  
  lcd_byte (c, LCD_CHR);       //# print first user defined character
  return;
}
//*---------------------------------------------------------------------------------------------------
//*  LCD blink at cursor (not implemented yet)
//*---------------------------------------------------------------------------------------------------
void LCDLib::Blink() {
  

  return;
}
//*---------------------------------------------------------------------------------------------------
//*  LCD not blink at cursor (not implemented yet)
//*---------------------------------------------------------------------------------------------------
void LCDLib::noBlink() {
  

  return;
}
//*---------------------------------------------------------------------------------------------------
//*  LCD print a float at current cursor
//*---------------------------------------------------------------------------------------------------
void LCDLib::typeFloat(float f)   {
  char buffer[20];
  sprintf(buffer, "%4.2f",  f);
  typeln(buffer);
}
//*---------------------------------------------------------------------------------------------------
//*  LCD print an integer at current cursor
//*---------------------------------------------------------------------------------------------------
void LCDLib::typeInt(int i)   {
  char a[20];
  sprintf(a, "%d",  i);
  typeln(a);
}
//*---------------------------------------------------------------------------------------------------
//*  LCD locate a precise memory address 0x80 LINE1 0xC0 LINE2
//*---------------------------------------------------------------------------------------------------

void LCDLib::lcdLoc(int line)   {
  lcd_byte(line, LCD_CMD);
}
//*---------------------------------------------------------------------------------------------------
//*  LCD set backlight
//*---------------------------------------------------------------------------------------------------
void LCDLib::backlight(boolean v) {

   if (v==true) {
     LCD_LIGHT=LCD_ON;
   } else {
     LCD_LIGHT=LCD_OFF;
   }

}
//*---------------------------------------------------------------------------------------------------
//*  LCD print char at current location (for char* use typeln for string use print)
//*---------------------------------------------------------------------------------------------------
void LCDLib::typeChar(char val)   {
  lcd_byte(val, LCD_CHR);
}
//*---------------------------------------------------------------------------------------------------
//*  LCD print a char*
//*---------------------------------------------------------------------------------------------------
void LCDLib::typeln(const char *s)   {

  while ( *s ) lcd_byte(*(s++), LCD_CHR);

}
//*---------------------------------------------------------------------------------------------------
//*  LCD send a single byte (structural method)
//*---------------------------------------------------------------------------------------------------

void LCDLib::lcd_byte(int bits, int mode)   {

  // mode = 1 for data, 0 for command
  int bits_high;
  int bits_low;

  // uses the two half byte writes to LCD
  bits_high = mode | (bits & 0xF0) | LCD_LIGHT ;
  bits_low = mode | ((bits << 4) & 0xF0) | LCD_LIGHT ;

  // High bits
  wiringPiI2CReadReg8(fd, bits_high);
  lcd_toggle_enable(bits_high);

  // Low bits
  wiringPiI2CReadReg8(fd, bits_low);
  lcd_toggle_enable(bits_low);
}
//*---------------------------------------------------------------------------------------------------
//*  LCD toggle ENABLE pin on LCD display
//*---------------------------------------------------------------------------------------------------
void LCDLib::lcd_toggle_enable(int bits)   {
  delayMicroseconds(500);
  wiringPiI2CReadReg8(fd, (bits | ENABLE));
  delayMicroseconds(500);
  wiringPiI2CReadReg8(fd, (bits & ~ENABLE));
  delayMicroseconds(500);
}

