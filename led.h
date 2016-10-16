
extern volatile uint8_t led_framebuf[8][4];
void update_led(void);
void set_led(uint8_t idx, char ch, uint8_t blink);

uint8_t led_fixup(uint8_t val);

