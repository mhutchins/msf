
#define SPI_SS	PB2
#define SPI_MOSI	PB3
#define SPI_SCK		PB5

#define SPI_BUF_LEN 10

extern volatile uint8_t SPI_buf[];
extern volatile uint8_t SPI_buf_w_idx;
extern volatile uint8_t SPI_buf_r_idx;


void spi_init(void);
void spi_queue(uint8_t value);
void spi_loop(void);

uint8_t spi_enqueue(uint8_t value);
uint8_t spi_dequeue(uint8_t *qval);

void spi_dumpqueue(void);

