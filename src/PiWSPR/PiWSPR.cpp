/**
 * PiWSPR.cpp 
 * Raspberry Pi based WSPR beacon
 *
 * This program turns the Raspberry pi into a WSPR beacon software able
 * to operate at the indicated frequency as a direct RF generator
 *
 * Created by Pedro E. Colla (lu7did@gmail.com)
 * Code excerpts from several packages:
 *     tune.cpp from rpitx package by Evariste Courjaud F5OEO
 *     wiringPi library (git clone git://git.drogon.net/wiringPi)
 *     WSPR-beacon by Alexander Fasching OE5TKM
 * ---------------------------------------------------------------------
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *  
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */
//----------------------------------------------------------------------------
//  includes
//----------------------------------------------------------------------------

//---- Generic includes

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <dirent.h>
#include <math.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

//---  VFO initial setup
typedef unsigned char byte;
typedef bool boolean;



#include <unistd.h>
//---- Program specific includes
#include "/home/pi/librpitx/src/librpitx.h"
#include "../lib/DDS.h"
#include "./PiWSPR.h"		// wspr definitions and functions

#define WSPR_TXF  14095600
#define VFO_START 14095600
#define WSPR_LENGTH    162
//----------------------------------------------------------------------------
//  Program parameter definitions
//----------------------------------------------------------------------------

const char   *PROGRAMID="PiWSPR";
const char   *PROG_VERSION="1.0";
const char   *PROG_BUILD="00";
const char   *COPYRIGHT="(c) LU7DID 2019";



//-------------------------------------------------------------------------------------------------
// Main structures
//-------------------------------------------------------------------------------------------------


//---- Generic memory allocations

int value=0;
int lastEncoded=0;
int counter=0;
int clkLastState=0; 
char hi[80];
byte memstatus=0;
byte ntimes=1;
int a;
int anyargs = 0;
float SetFrequency=VFO_START;
float ppm=1000.0;
struct sigaction sa;
bool running=true;
byte keepalive=0;
bool first=true;
char port[80];
byte gpio=GPIO04;

char wspr_message[20];          // user beacon message to encode
unsigned char wspr_symbols[WSPR_LENGTH] = {};
unsigned long tuning_words[WSPR_LENGTH];
int i;
double centre_freq;

void cbkDDS();
DDS    dds(cbkDDS);

//=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
//                              ROUTINE STRUCTURE
//=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
//--------------------------------------------------------------------------
// Callback for DDS pointers
//--------------------------------------------------------------------------
void cbkDDS() {

}
//-------------------------------------------------------------------------------------------------
// print_usage
// help message at program startup
//-------------------------------------------------------------------------------------------------
void print_usage(void)
{
fprintf(stderr,"\n\
Usage:\n\
\t-f frequency Hz(50000 Hz to 1500000000 Hz),\n\
\t-p set clock ppm instead of ntp adjust\n\
\t-c set callsign (ej LU7DID)\n\
\t-l set locator (ej GF05)\n\
\t-d set power in dBm\n\
\t-n set number of times to emit (0 indefinite)\n\
\t-g set GPIO port (4 or 20)\n\
\t-h help (this help).\n\n\
\t e.g.: PiWSPR -c LU7DID -l GF05 -d 20 -f 14095600 -n 1 -g 20\n\
\n");
}
//---------------------------------------------------------------------------------------------
// Signal handlers, SIGALRM is used as a timer, all other signals means termination
//---------------------------------------------------------------------------------------------
static void terminate(int num)
{
    printf("\n received signal(%d %s)\n",num,strsignal(num));
    running=false;
   
}
//---------------------------------------------------------------------------------------------
/*
WSPR encoding module:
Thanks to K1JT, G4JNT and PE1NZZ for publishing
helping infos.

Encoding process is in 5 steps:
   * bits packing of user message in 50 bits
   * store the 50 bits dans 11 octets (88 bits and only 81 useful)
   * convolutionnal encoding with two pariy generators (-> 162 bits)
   * interleaving of the 162 bits with bit-reverse technique
   * synchronisation with a psudo-random vector to obtain the
      162 symbols defining one frequency of 4.

 F8CHK 29/03/2011                              */
//---------------------------------------------------------------------------------------------
void Code_msg (char usr_message[], unsigned long int *N, unsigned long int *M)
{
  unsigned long int n, m;
  unsigned int i, j, power, callsign_length;

  char callsign[7] = "",	// callsign string
    locator[5] = "",		// locator string
    power_str[3] = "";		// power string


  strcpy (callsign, "      ");	// filling with spaces

  i = 0;
  while (usr_message[i] != ' ')
    {
      callsign[i] = islower(usr_message[i])?toupper(usr_message[i]):usr_message[i];	// extract callsign
      i++;
    }
  callsign_length = i;

  i++;
  j = 0;
  while (usr_message[i] != ' ')
    locator[j++] = islower(usr_message[i])?toupper(usr_message[i++]):usr_message[i++];	// extract locator
  locator[j] = 0;

  i++;
  j = 0;
  while (usr_message[i] != 0)
    power_str[j++] = usr_message[i++];	// extract power
  power_str[j] = 0;

  power = atoi (power_str);	// power needs to be an integer

  printf("Call: %s / Locator: %s / Power: %ddBm\n", callsign, locator, power);

  // Place a space in first position if third character is not a digit
  if (!isdigit (callsign[2]))
    {
      for (i = callsign_length; i > 0; i--)
	callsign[i] = callsign[i - 1];
      callsign[0] = ' ';
    }

  // callsign encoding:  
  // numbers have a value between 0 and 9 
  // and letters a value between 10 and 35
  // spaces a value of 36
  n = (callsign[0] >= '0'
       && callsign[0] <= '9' ? callsign[0] - '0' : callsign[0] ==
       ' ' ? 36 : callsign[0] - 'A' + 10);
  n = n * 36 + (callsign[1] >= '0'
		&& callsign[1] <= '9' ? callsign[1] - '0' : callsign[1] ==
		' ' ? 36 : callsign[1] - 'A' + 10);
  n = n * 10 + (callsign[2] - '0');	// only number (0-9)
  n = 27 * n + (callsign[3] == ' ' ? 26 : callsign[3] - 'A');	// only space or letter
  n = 27 * n + (callsign[4] == ' ' ? 26 : callsign[4] - 'A');
  n = 27 * n + (callsign[5] == ' ' ? 26 : callsign[5] - 'A');

  // Locator encoding
  m =
    (179 - 10 * (locator[0] - 65) - (locator[2] - 48)) * 180 +
    10 * (locator[1] - 65) + locator[3] - 48;

  // Power encoding
  m = m * 128 + power + 64;

  *N = n;
  *M = m;
}
//---------------------------------------------------------------------------------------------
// Pack_msg
//---------------------------------------------------------------------------------------------
void Pack_msg (unsigned long int N, unsigned long int M, unsigned char c[])
{
// Bit packing
// Store in 11 characters because we need 81 bits for FEC correction
  c[0] = N >> 20;		// Callsign
  c[1] = N >> 12;
  c[2] = N >> 4;
  c[3] = N;
  c[3] = c[3] << 4;

  c[3] = c[3] | (M >> 18);	// locator and power
  c[4] = M >> 10;
  c[5] = M >> 2;
  c[6] = M & 0x03;
  c[6] = c[6] << 6;

  c[7] = 0;			// always at 0
  c[8] = 0;
  c[9] = 0;
  c[10] = 0;
}
//---------------------------------------------------------------------------------------------
// Generate_parity
//---------------------------------------------------------------------------------------------
void Generate_parity (unsigned char c[], unsigned char symbols[])
{
  unsigned long int Reg0 = 0,	// 32 bits shift register
    Reg1 = 0, result0, result1;
  int count1,			// to count the number
    count2,			// of bits at one
    bit_result = 0, i, j, k, l;

  l = 0;
  for (j = 0; j < 11; j++)	// each byte
    {
      for (i = 7; i >= 0; i--)
	{
	  Reg0 = (Reg0 << 1);
	  Reg0 = Reg0 | (c[j] >> i);	// each bit
	  Reg1 = Reg0;

	  result0 = Reg0 & POLYNOM_1;	// first polynom
	  count1 = 0;

	  for (k = 0; k < 32; k++)	// how many bit at one?
	    {
	      bit_result = result0 >> k;
	      if ((bit_result & 0x01) == 1)
		count1++;
	    }
	  if (count1 % 2 == 1)	// if number of one is odd
	    symbols[l] = 1;	// parity = 1
	  l++;

	  result1 = Reg1 & POLYNOM_2;	// second polynom
	  count2 = 0;

	  for (k = 0; k < 32; k++)	// how many bit at one?
	    {
	      bit_result = result1 >> k;
	      if ((bit_result & 0x01) == 1)
		count2++;
	    }
	  if (count2 % 2 == 1)	// if number of one is odd
	    symbols[l] = 1;	// parity = 1
	  l++;
	}			// end of each bit (32) loop
    }				// end of each byte (11) loop
}
//---------------------------------------------------------------------------------------------
// Interleave
//---------------------------------------------------------------------------------------------
void Interleave (unsigned char symbols[], unsigned char symbols_interleaved[])
{
  int i, j, k, l, P;

  P = 0;
  while (P < WSPR_LENGTH)
    {
      for (k = 0; k <= 255; k++)	// bits reverse, ex: 0010 1110 --> 0111 0100
	{
	  i = k;
	  j = 0;
	  for (l = 7; l >= 0; l--)	// hard work is done here...
	    {
	      j = j | (i & 0x01) << l;
	      i = i >> 1;
	    }
	  if (j < WSPR_LENGTH)
	    symbols_interleaved[j] = symbols[P++];	// range in interleaved table
	}
    }				// end of while, interleaved table is full
}
//---------------------------------------------------------------------------------------------
// Synchronise
//---------------------------------------------------------------------------------------------
void Synchronise (unsigned char symbols_interleaved[],
	     unsigned char symbols_wspr[])
{
  unsigned int sync_word [WSPR_LENGTH]={
    1,1,0,0,0,0,0,0,1,0,0,0,1,1,1,0,0,0,1,0,0,1,0,1,1,1,1,0,0,0,0,0,0,0,1,0,0,1,0,1,0,0,
    0,0,0,0,1,0,1,1,0,0,1,1,0,1,0,0,0,1,1,0,1,0,0,0,0,1,1,0,1,0,1,0,1,0,1,0,0,1,0,0,1,0,
    1,1,0,0,0,1,1,0,1,0,1,0,0,0,1,0,0,0,0,0,1,0,0,1,0,0,1,1,1,0,1,1,0,0,1,1,0,1,0,0,0,1,
    1,1,0,0,0,0,0,1,0,1,0,0,1,1,0,0,0,0,0,0,0,1,1,0,1,0,1,1,0,0,0,1,1,0,0,0
  };

  int i;

  for (i = 0; i < WSPR_LENGTH; i++)
    symbols_wspr[i] = sync_word[i] + 2 * symbols_interleaved[i];
}
//---------------------------------------------------------------------------------------------
// code_wspr
//---------------------------------------------------------------------------------------------
void code_wspr (char* wspr_message, unsigned char* wspr_symbols)
{
  unsigned char symbols_parity[WSPR_LENGTH] = "",	// contains 2*81 parity bits
    symbols_interleaved[WSPR_LENGTH] = "",		// contains parity bits after interleaving
    c_packed[11];		// for bit packing

  unsigned long N,		// for callsign
    M;				// for locator and power


  Code_msg (wspr_message, &N, &M);
  Pack_msg (N, M, c_packed);
  Generate_parity (c_packed, symbols_parity);
  Interleave (symbols_parity, symbols_interleaved);
  Synchronise (symbols_interleaved, wspr_symbols);

}
//---------------------------------------------------------------------------------------------
// calculate_tuning_info
//---------------------------------------------------------------------------------------------
void calculate_tuning_info(tuning_data* tuning_info)
{
  double divisor; 
  unsigned long decimal_part;
  unsigned long fractional_part;
  double actual_divisor;

  divisor = (double)500000000/tuning_info->requested;
  decimal_part = (unsigned long) divisor;
  fractional_part = (divisor - decimal_part) * (1 << 12);
  tuning_info->tuning_word = decimal_part * (1 << 12) + fractional_part;
  actual_divisor = (double)tuning_info->tuning_word / (float)(1 << 12);

  tuning_info->actual = (double)500000000 / actual_divisor;
}
//---------------------------------------------------------------------------------------------
// sym_to_tuning_words
//---------------------------------------------------------------------------------------------
void sym_to_tuning_words(double base_freq, unsigned char* wspr_symbols, unsigned long* tuning_words)
{
  int i;
  double symbol_freq;
  tuning_data tuning_info[4];


  for (i = 0; i < 4; i++)
  {
    symbol_freq = base_freq + (i-2) * WSPR_OFFSET;
    tuning_info[i].requested = symbol_freq;
    calculate_tuning_info(&tuning_info[i]);
    printf("Symbol %d: Target freq=%fHz, Actual freq=%fHz, Error=%fHz, Tuning Word=%lx\n", i, symbol_freq, tuning_info[i].actual, symbol_freq-tuning_info[i].actual, tuning_info[i].tuning_word);
  }

  for (i = 0; i < WSPR_LENGTH; i++)
  {
    tuning_words[i] = tuning_info[wspr_symbols[i]].tuning_word;
  }
}
//---------------------------------------------------------------------------------------------
// wsprTXtone
// compute frequency to transmit and change DDS to match it
//---------------------------------------------------------------------------------------------
void wsprTXtone(int t) {
    if ((t >= 0) && (t <= 3) ) {
       float frequency=(WSPR_TXF + (t * 1.4648));
       printf("Code(%d) Frequency(%10.2f)\n",t,frequency);
    }
}
//---------------------------------------------------------------------------------------------
// MAIN Program
//---------------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{

//--- Initial presentation

  sprintf(hi,"%s Version %s Build(%s) %s\n",PROGRAMID,PROG_VERSION,PROG_BUILD,COPYRIGHT);
  printf(hi);

//  if(argc != 5){
//    printf("Usage: wspr-pi <callsign> <locator> <power in dBm> <frequency in Hz>\n");
//    return 1;
//  }

//--- Parse arguments

   while(1)
        {
                a = getopt(argc, argv, "f:ed:hs:g:p:c:l:");
        
                if(a == -1) 
                {
                        if(anyargs) {
                           break; 
                        } else {
                           a='h'; //print usage and exit
                        }
                }
                anyargs = 1;    

                switch(a)
                {
                case 'f': // Frequency
                        SetFrequency = atof(optarg);
                        break;
                case 'g': // Frequency
              
                        gpio = atoi(optarg);
                        if (gpio != GPIO04 && gpio != GPIO20) {
                           sprintf(port,optarg);
                           fprintf(stderr,"Invalid selection for GPIO(%s), must be 4 or 20\n",optarg);
                           break;
                        }
                        sprintf(port,optarg);
                        fprintf(stderr, "GPIO port set to:%s\n", optarg);
                        break;
                case 'p': //ppm
                        ppm=atof(optarg);
                        break;  
                case 'h': // help
                        print_usage();
                        exit(1);
                        break;
                case 'd': //
                        break;
                case 's': //serial port
                        sprintf(port,optarg);
                        fprintf(stderr, "serial port:%s\n", optarg);
                        break;
                case -1:
                break;
                case '?':
                        if (isprint(optopt) )
                        {
                           fprintf(stderr, "PiWSPR: unknown option `-%c'.\n", optopt);
                        }
                        else
                        {
                           fprintf(stderr, "PiWSPR: unknown option character `\\x%x'.\n", optopt);
                        }
                        print_usage();

                        exit(1);
                        break;
                default:
                        print_usage();
                        exit(1);
                        break;
                }
        }

//--- Define the rest of the signal handlers, basically as termination exceptions

    for (int i = 0; i < 64; i++) {

        if (i != SIGALRM && i != 17 && i != 28 ) {
           std::memset(&sa, 0, sizeof(sa));
           sa.sa_handler = terminate;
           sigaction(i, &sa, NULL);
        }
    }


//--- Generate DDS (code excerpt mainly from tune.cpp by Evariste Courjaud F5OEO

    dds.gpio=gpio;
    dds.setppm(ppm);
    //dds.ppm=ppm;
    //dds.open(SetFrequency);

    sprintf(wspr_message, "%s %s %s", argv[1], argv[2], argv[3]);
    printf("Sending |%s|\n", wspr_message);

    code_wspr(wspr_message, wspr_symbols);

    for (i = 0; i < WSPR_LENGTH; i++) {
      printf("%d, ", wspr_symbols[i]);
    }
    printf("\n");


    for (int j=0;j<WSPR_LENGTH;j++) {
        int t=wspr_symbols[j];
        if ((t >= 0) && (t <= 3) ) {
           float frequency=(WSPR_TXF + (t * 1.4648));
           dds.set(frequency);
           printf("Code(%d) Frequency(%10.2f)\n",t,frequency);
        }

       //wsprTXtone( wspr_symbols[j] );
       usleep(683000); 
    }
  dds.close();
  usleep(100000);

  printf("Finalizing WSPR frame\n");
  usleep(10000);
  printf("\nProgram terminated....\n");
  exit(0);
}
