void binprint(uint16_t data, uint8_t bits);
unsigned char safe_i2c_start(unsigned char addr);
void safe_i2c_start_wait(unsigned char addr);
void safe_i2c_stop(void);