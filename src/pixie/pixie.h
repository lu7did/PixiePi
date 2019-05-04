//17 pins / 2 pins per encoder = 8 maximum encoders
struct encoder
{
    int pin_a;
    int pin_b;
    volatile long value;
    volatile int lastEncoded;
};
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

//*-- Added by Lewis Loflin (LCD related)

//void lcd_init(void);
//void lcd_byte(int bits, int mode);
//void lcd_toggle_enable(int bits);
//void typeInt(int i);
//void typeFloat(float myFloat);
//void lcdLoc(int line);             //move cursor
//void ClrLcd(void);                 // clr LCD return home
//void typeln(const char *s);
//void typeChar(char val);
void showFreq();                       //Prototype fur display used on callback
void setDDSFrequency();


