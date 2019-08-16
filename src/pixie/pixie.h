//17 pins / 2 pins per encoder = 8 maximum encoders
struct encoder
{
    int pin_a;
    int pin_b;
    volatile long value;
    volatile int lastEncoded;
};

//*----------------------------------------------------------------------------
//* Special macro definitions to adapt for previous code on the Arduino board
//*----------------------------------------------------------------------------
typedef unsigned char byte;
typedef bool boolean;

//*--- Function prototypes
void setWord(unsigned char* SysWord,unsigned char  v, bool val);
bool getWord (unsigned char SysWord, unsigned char v);

void VfoUpdate();
void StepUpdate();
void ShiftUpdate();
void SplitUpdate();
void KeyerUpdate();
void LockUpdate();
void WatchDogUpdate();
void BackLightUpdate();
void setDDSFreq();
void ModeUpdate();
void DriverUpdate();
void SpeedUpdate();
void RitUpdate();

//*-- Added by Lewis Loflin (LCD related)

void showFreq();                       //Prototype fur display used on callback
void setDDSFrequency();

void setPTT(bool t);
typedef unsigned char byte;
typedef bool boolean;
typedef void (*CALLBACK)();

// GPIO pins
// set to 0 to use the PI's hw:0 audio out for sidetone
#define SIDETONE_GPIO 19 // this is in wiringPi notation

// Define output GPIO pin

#define KEYER_OUT_GPIO 12
#define LEFT_PADDLE_GPIO 13
#define RIGHT_PADDLE_GPIO 15

#define KEY_DOWN 0x01
#define KEY_UP 0x00

//#define SPEED_GPIO 19

#define KEYER_STRAIGHT 0
#define KEYER_MODE_A 1
#define KEYER_MODE_B 2

#define NSEC_PER_SEC (1000000000)
enum {
    CHECK = 0,
    PREDOT,
    PREDASH,
    SENDDOT,
    SENDDASH,
    DOTDELAY,
    DASHDELAY,
    DOTHELD,
    DASHHELD,
    LETTERSPACE,
    EXITLOOP
};

#define DELAY_SAVE	800000
#define CW_SHIFT        600
#define MAXPOWER        7

//*--- Encoder pin definition


#define ENCODER_CLK 17
#define ENCODER_DT  18
#define ENCODER_SW  27



//*---  VFO initial setup

#define VFO_START         7000000
#define VFO_END           7299000
#define VFO_BAND_START          3
#define ONESEC               1000

#define VFO_DELAY               1
#define BACKLIGHT_DELAY        60

#define GPIO04                  4
#define GPIO20                 20

//*----- Master Timer and Event flagging (TSW)

#define FT1       0B00000001
#define FT2       0B00000010
#define FT3       0B00000100
#define FTU       0B00001000
#define FTD       0B00010000
#define FVFO      0B00100000
#define FDOG      0B01000000
#define FBCK      0B10000000

//*--- Master System Word (MSW)

#define CMD       0B00000001
#define GUI       0B00000010
#define PTT       0B00000100
#define DRF       0B00001000
#define DOG       0B00010000
#define LCLK      0B00100000
#define SQL       0B01000000
#define BCK       0B10000000

//*----- UI Control Word (USW)

#define BBOOT     0B00000001
#define BMULTI    0B00000010
#define BCW       0B00000100
#define BCCW      0B00001000
#define SQ        0B00010000
#define MIC       0B00100000
#define KDOWN     0B01000000
#define BUSY      0B10000000       //Used for Squelch Open in picoFM and for connected to DDS on sinpleA
#define CONX      0B10000000

//*----- Joystick Control Word (JSW)

#define JLEFT     0B00000001
#define JRIGHT    0B00000010
#define JUP       0B00000100
#define JDOWN     0B00001000
#define XVFO      0B00010000


//#define SMETERMAX  15
#define MINSWPUSH  10
#define MAXSWPUSH  2000
#define MINENCLAP   2
#define TWOSECS     2
#define ONESECS     1
#define MSEC100  1000

#define MAXRIT     1000
#define MINRIT    -1000
#define RITSTEP    100
#define RITSTEPD  -100

#define CATBAUD    4800

#define _NOP        (byte)0
