
#define MAX7219_NOOP            0x00
#define MAX7219_DIGIT0          0x01
#define MAX7219_DIGIT1          0x02
#define MAX7219_DIGIT2          0x03
#define MAX7219_DIGIT3          0x04
#define MAX7219_DIGIT4          0x05
#define MAX7219_DIGIT5          0x06
#define MAX7219_DIGIT6          0x07
#define MAX7219_DIGIT7          0x08
#define MAX7219_DECODE          0x09
#define MAX7219_INTENSITY       0x0A
#define MAX7219_SCANLIMIT       0x0B
#define MAX7219_SHUTDOWN        0x0C
#define MAX7219_TEST            0x0F

#define D_A	6
#define D_B	5
#define D_C	4
#define D_D	3
#define D_E	2
#define D_F	1
#define D_G	0

void max7219(uint8_t addr, uint8_t data);
uint8_t getled(unsigned char ch);
void printled(uint8_t bitval);
