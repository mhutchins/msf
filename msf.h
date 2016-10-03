extern volatile uint16_t tick;
extern volatile uint16_t offset;
extern volatile uint8_t sync_flag;
extern volatile uint8_t sec;
extern uint8_t msf_bit_a[];
extern uint8_t msf_bit_b[];
extern volatile int hour, min, mon, dom, dow, year;
extern time_t msf_time[];


void binprint(uint16_t data, uint8_t len);
void clear_msf(uint8_t msf[]);
time_t decode(void);
uint8_t get_msf_bit(uint8_t msf[], uint8_t period);
uint8_t getparity(uint8_t start, uint8_t end);
void set_msf_bit(uint8_t msf[], uint8_t period, uint8_t val);
void timer_init(void);
