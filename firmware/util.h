#ifndef _UTIL_H
#define _UTIL_H

struct bits {
  uint8_t b0:1;
  uint8_t b1:1;
  uint8_t b2:1;
  uint8_t b3:1;
  uint8_t b4:1;
  uint8_t b5:1;
  uint8_t b6:1;
  uint8_t b7:1;
} __attribute__((__packed__));

#define BIT(name,pin,reg) \
 ((*(volatile struct bits*)&reg##name).b##pin)


// Logic constants
#define FALSE 0
#define TRUE !FALSE
#define HIGH 1
#define LOW 0
#define OUTPUT 1
#define INPUT 0
#define PULLUP_ON 1
#define PULLUP_OFF 0 

#define WORD(h, l) ((h << 8) | l)
#endif
