/*
 * PixiePi.c
 * Raspberry Pi based transceiver
 *
 *
 * Created by Pedro E. Colla (lu7did@gmail.com)
 * Code excerpts from several packages:
 *    I2C Interface to 16X2 LCD 
 *       Adafruit's python code for CharLCDPlate
 *       Interface I2C to C by Lewis Loflin www.bristolwatch.com
 *       Using wiringPi by Gordon Henderson
 *       https://projects.drogon.net/raspberry-pi/wiringpi/
 *       This must be installed first.
 *       Port over lcd_i2c.py to C and added improvements.
 *       Supports 16x2 and 20x4 screens.
 * =========================================================================
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * =========================================================================
 * There is no warranty of any kind -- use it at your own risk --
 */

#include <wiringPiI2C.h>
#include <wiringPi.h>
#include <stdlib.h>
#include <stdio.h>

// Define some device parameters
#define I2C_ADDR   0x27 // I2C device address

// Define some device constants
#define LCD_CHR  1 // Mode - Sending data
#define LCD_CMD  0 // Mode - Sending command

#define LINE1  0x80 // 1st line
#define LINE2  0xC0 // 2nd line

#define LCD_BACKLIGHTON  0x08
#define LCD_BACKLIGHTOFF 0x00
#define LCD_BACKLIGHT   LCD_BACKLIGHTON  // On

#define ENABLE  0b00000100 // Enable bit

//*-- Added by Lewis Loflin

void lcd_init(void);
void lcd_byte(int bits, int mode);
void lcd_toggle_enable(int bits);
void typeInt(int i);
void typeFloat(float myFloat);
void lcdLoc(int line); //move cursor
void ClrLcd(void); // clr LCD return home
void typeln(const char *s);
void typeChar(char val);

int fd;  // seen by all subroutines

/*--------------------------------------------------------------------
 * main
 *
*/
int main()   {

  if (wiringPiSetup () == -1) exit (1);

  fd = wiringPiI2CSetup(I2C_ADDR);
  
  //printf("fd = %d ", fd);

  lcd_init(); // setup LCD
  char array1[] = "PixiePi LCD Test";

  while (1)   {

    lcdLoc(LINE1);
    typeln("Test WiringPi");
    lcdLoc(LINE2);
    typeln("PixiePi");

    ClrLcd();
    delay(1000);

    delay(2000);
    ClrLcd();
    lcdLoc(LINE1);
    typeln("I2c  Programmed");
    lcdLoc(LINE2);
    typeln("in C not Python.");

    delay(2000);
    ClrLcd();
    lcdLoc(LINE1);
    typeln("LCD Testing");
    lcdLoc(LINE2);
    typeln("HF Transceiver");

    delay(2000);
    ClrLcd();
    lcdLoc(LINE1);
    typeln(array1);

    delay(2000);
    ClrLcd(); // defaults LINE1
    typeln("Integer  ");
    int value = 20125;
    typeInt(value);

    delay(2000);
    lcdLoc(LINE2);
    typeln("Float");
    float FloatVal = 10045.25989;
    typeFloat(FloatVal);
    delay(2000);
  }

  return 0;

}


// float to string
void typeFloat(float myFloat)   {
  char buffer[20];
  sprintf(buffer, "%4.2f",  myFloat);
  typeln(buffer);
}

// int to string
void typeInt(int i)   {
  char array1[20];
  sprintf(array1, "%d",  i);
  typeln(array1);
}

// clr lcd go home loc 0x80
void ClrLcd(void)   {
  lcd_byte(0x01, LCD_CMD);
  lcd_byte(0x02, LCD_CMD);
}

// go to location on LCD
void lcdLoc(int line)   {
  lcd_byte(line, LCD_CMD);
}

// out char to LCD at current position
void typeChar(char val)   {

  lcd_byte(val, LCD_CHR);
}


// this allows use of any size string
void typeln(const char *s)   {

  while ( *s ) lcd_byte(*(s++), LCD_CHR);

}

void lcd_byte(int bits, int mode)   {

  //Send byte to data pins
  // bits = the data
  // mode = 1 for data, 0 for command
  int bits_high;
  int bits_low;
  // uses the two half byte writes to LCD
  bits_high = mode | (bits & 0xF0) | LCD_BACKLIGHT ;
  bits_low = mode | ((bits << 4) & 0xF0) | LCD_BACKLIGHT ;

  // High bits
  wiringPiI2CReadReg8(fd, bits_high);
  lcd_toggle_enable(bits_high);

  // Low bits
  wiringPiI2CReadReg8(fd, bits_low);
  lcd_toggle_enable(bits_low);
}

void lcd_toggle_enable(int bits)   {
  // Toggle enable pin on LCD display
  delayMicroseconds(500);
  wiringPiI2CReadReg8(fd, (bits | ENABLE));
  delayMicroseconds(500);
  wiringPiI2CReadReg8(fd, (bits & ~ENABLE));
  delayMicroseconds(500);
}


void lcd_init()   {
  // Initialise display
  lcd_byte(0x33, LCD_CMD); // Initialise
  lcd_byte(0x32, LCD_CMD); // Initialise
  lcd_byte(0x06, LCD_CMD); // Cursor move direction
  lcd_byte(0x0C, LCD_CMD); // 0x0F On, Blink Off
  lcd_byte(0x28, LCD_CMD); // Data length, number of lines, font size
  lcd_byte(0x01, LCD_CMD); // Clear display
  delayMicroseconds(500);
}

