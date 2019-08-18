/* Functions for wspr encoding */

#ifndef __WSPR_H
#define __WSPR_H

void Code_msg(char[], unsigned long int*, unsigned long int*);      // encode callsign, locator and power
void Pack_msg(unsigned long int, unsigned long int, unsigned char[]);// packed 50 bits in 11 bytes
void Generate_parity(unsigned char[], unsigned char[]);  // generate 162 parity bits
void Interleave( unsigned char[], unsigned char[]);  // interleave the 162 parity bits
void Synchronise(unsigned char[], unsigned char[]);  // synchronize with a pseudo random pattern

void code_wspr(char* wspr_message, unsigned char* wspr_symbol);		// encode the wspr message

#endif

#define KEYER_OUT_GPIO    12
#define WSPR_RAND_OFFSET 80
#define VFO_START 14095600
#define GPIO04     4
#define GPIO20    20
#define GPIO12    12
#define WSPR_SHIFT    1400
#define WSPR_BAND      100

