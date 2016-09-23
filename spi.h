
#define SPI_SS	PB2
#define SPI_MOSI	PB3
#define SPI_SCK		PB5

#define SPI_BUF_LEN 10

#define MAX7219_NOOP		0x00
#define MAX7219_DIGIT0		0x01
#define MAX7219_DIGIT1		0x02
#define MAX7219_DIGIT2		0x03
#define MAX7219_DIGIT3		0x04
#define MAX7219_DIGIT4		0x05
#define MAX7219_DIGIT5		0x06
#define MAX7219_DIGIT6		0x07
#define MAX7219_DIGIT7		0x08
#define MAX7219_DECODE		0x09
#define MAX7219_INTENSITY	0x0A
#define MAX7219_SCANLIMIT	0x0B
#define MAX7219_SHUTDOWN	0x0C
#define MAX7219_TEST		0x0F


extern volatile uint8_t SPI_buf[];
extern volatile uint8_t SPI_buf_w_idx;
extern volatile uint8_t SPI_buf_r_idx;


void spi_init(void);
void spi_queue(uint8_t value);
void spi_loop(void);

uint8_t spi_enqueue(uint8_t value);
uint8_t spi_dequeue(uint8_t *qval);

void spi_dumpqueue(void);

void max7219(uint8_t addr, uint8_t data);

