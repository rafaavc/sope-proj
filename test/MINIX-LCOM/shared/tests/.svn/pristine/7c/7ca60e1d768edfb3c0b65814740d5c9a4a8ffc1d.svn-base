#include <lcom/lcf.h>
#include <lcom/timer.h>

#include <stdint.h>

#include "i8254.h"
#include "timer.h"

//Global counter incremented in timer_int_handler()
int counter = 0;
//Global variable used in subscribe and unsubscribe int
int bit_id;

int (timer_set_frequency)(uint8_t timer, uint32_t freq) {
  //Changes the operating frequency of a timer.
  //Must use the read-back command so that it does not change the 4 LSBs (mode and BCD/binary) of the timer's control word.

  if (timer != 0 && timer != 1 && timer != 2) return 1; //failure state
  if (freq > TIMER_FREQ || freq < 19) return 1; //prevent errors when calculating TIMER_FREQ/frequency, i8254 only works with 16bits

  uint8_t command; //READ BACK COMMAND
  if (timer_get_conf(timer, &command)) return 1; //SAVE READ BACK COMMAND STATUS IN VARIABLE

  uint8_t lsb, msb; //Used to get least and most significant bytes
  uint16_t ratio = TIMER_FREQ / freq; //get real frequency

  if (util_get_LSB(ratio, &lsb)) return 1;
  if (util_get_MSB(ratio, &msb)) return 1;

  uint8_t time_bin = timer << 6; //places timer bits in counter place
  uint8_t ctrlWord = (command & (BIT(0)|BIT(1)|BIT(2)|BIT(3))) | TIMER_LSB_MSB | time_bin; //saves command lsb and puts bits 4 and 5 to one

  if (sys_outb(TIMER_CTRL, ctrlWord)) return 1;
  if (sys_outb(TIMER_0 + timer, lsb)) return 1;
  if (sys_outb(TIMER_0 + timer, msb)) return 1;

  return 0;
}

int (timer_subscribe_int)(uint8_t *bit_no) {
  //bit_no is preserved
  bit_id = *bit_no;
  if (sys_irqsetpolicy(TIMER0_IRQ, IRQ_REENABLE, &bit_id)) return 1;

  //no need to use sys_irqenable as sys_irqsetpolicy() already enables the corresponding interrupt
  return 0;
}

int (timer_unsubscribe_int)() {
  if (sys_irqrmpolicy(&bit_id)) return 1;

  return 0;
}

void (timer_int_handler)() {
  counter++;
}

int (timer_get_conf)(uint8_t timer, uint8_t *st) {
  if (timer != 0 && timer != 1 && timer != 2) return 1; //failure state

  uint8_t flag_bits = TIMER_RB_CMD | TIMER_RB_COUNT_ | TIMER_RB_SEL(timer); //build READ BACK command
  sys_outb(TIMER_CTRL, flag_bits);
  util_sys_inb(TIMER_0 + timer, st);

  return 0;
}

int (timer_display_conf)(uint8_t timer, uint8_t st, enum timer_status_field field) {

  union timer_status_field_val uniao;

  //VARIABLES USED TO GET INITIALIZATION AND OPERATING MODES
  uint8_t inmode_bits;
  uint8_t opmode_bits;

  switch(field){
    case 0: //tsf_all
      uniao.byte = st;
      break;

    case 1: //tsf_intial - initialization Mode
      inmode_bits = ((st & TIMER_LSB_MSB) >> 4); //CHECK BITS FOR INITIALIZATION MODE

      switch(inmode_bits){
        case 1: //LSB_only - 01
          uniao.in_mode = LSB_only;
          break;
        case 2: //MSB_only - 10
          uniao.in_mode = MSB_only;
          break;
        case 3: //MSB_after_LSB - 11
          uniao.in_mode = MSB_after_LSB;
          break;
        default: //INVAL_val - invalid initialization mode
          uniao.in_mode = INVAL_val;
          break;
      }
      break;

    case 2: //tsf_mode - Operating Mode
      opmode_bits = (st & (BIT(1)|BIT(2)|BIT(3))) >> 1; //CHECK BITS FOR OPERATING MODE
      uniao.count_mode = opmode_bits;
      //6 -> mode 2
      //7 -> mode 3
      switch(opmode_bits){
        case 6: //For 'compatibility with future Intel products'.
          uniao.count_mode = 2;
          break;
        case 7: //For 'compatibility with future Intel products'.
          uniao.count_mode = 3;
          break;
      }
      break;
    case 3: //tsf_base - Counting base
      uniao.bcd = (st & TIMER_BCD);
      break;

    default:
      return 1;
  }
  //returns 0 in case of success and a different integer otherwise
  return timer_print_config(timer, field, uniao);
}
