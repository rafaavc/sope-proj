#include <lcom/lcf.h>

#include <stdint.h>

#include "macro.h"

//#define LAB3
uint32_t cnt = 0; //used to count sys_inb calls

int(util_get_LSB)(uint16_t val, uint8_t *lsb) {
  if (lsb == NULL) return 1;

  *lsb = val & (0xFF);

  return 0;
}

int(util_get_MSB)(uint16_t val, uint8_t *msb) {
  if (msb == NULL) return 1;

  *msb = val >> 8;

  return 0;
}

int (util_sys_inb)(int port, uint8_t *value) {
  if (value == NULL) return 1;

  uint32_t *call;
  call = malloc(sizeof(uint32_t)); //allocate memory for the created variable

  sys_inb(port, call);
  #ifdef LAB3
  cnt++; //increment counter to sys_inb calls
  #endif

  *value = (uint8_t) *call;

  free(call); //free memory previously allocated
  return 0;
}

