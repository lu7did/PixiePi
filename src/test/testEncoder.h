//17 pins / 2 pins per encoder = 8 maximum encoders
struct encoder
{
    int pin_a;
    int pin_b;
    volatile long value;
    volatile int lastEncoded;
};


