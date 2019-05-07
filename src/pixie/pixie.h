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

//*-- Added by Lewis Loflin (LCD related)

void showFreq();                       //Prototype fur display used on callback
void setDDSFrequency();


