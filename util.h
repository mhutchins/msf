extern char dayname[8][4];
void binprint(uint16_t data, uint8_t bits);
unsigned char safe_i2c_start(unsigned char addr);
void safe_i2c_start_wait(unsigned char addr);
void safe_i2c_stop(void);
uint8_t Uint8ToBcd(uint8_t ival);
uint8_t BcdToUint8(uint8_t val);
uint8_t BcdToBin24Hour(uint8_t bcdHour);


void readalarm(uint8_t idx);
void writealarm(uint8_t idx);

extern uint8_t track;

