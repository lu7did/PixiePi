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



