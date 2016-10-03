
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

void max7219(uint8_t addr, uint8_t data);
void displaytime(time_t *clock);


