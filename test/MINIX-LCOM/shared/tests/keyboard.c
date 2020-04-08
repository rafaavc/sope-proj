#include <lcom/lcf.h>

#include <stdint.h>

#include "macro.h"
#include "keyboard.h"

//Global variables
int bit_id_kbc;
uint8_t stat = 0; //where status register is going to be saved
uint8_t code = 0; //scancode
int ih_fail = 0; //checks failure state of void function kbc_ih

int (kbc_subscribe_int)(uint8_t *bit_no) {
  //bit_no is preserved
  bit_id_kbc = (int)*bit_no;
  return (sys_irqsetpolicy(KBD_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &bit_id_kbc));
}

int (kbc_unsubscribe_int)() {
  return sys_irqrmpolicy(&bit_id_kbc);
}

int (get_scancode)(){

  while( 1 ) {
    if (util_sys_inb(STAT_REG, &stat)) return 1; //gets status register value
    if( stat & OBF ) { //checks if output buffer is set
      if (((stat &(PAR_ERR | TO_ERR)) == 0)) { //Parity and Time Out errors check
        if(util_sys_inb(OUT_BUF, &code)) return 1; //Reads scancode
        else return 0;
      } 
      else return 1;
    }
  }
  return 0;
}
void (kbc_ih)() {
  //The handler should read the status from the status register, to check if there was some communications error
  //and to read the scancode byte from the output buffer.
  ih_fail = get_scancode();
}
int enable_interrupts(){

  uint8_t command;

  //WHILE LOOPS MADE TO CHECK IBF AND OBF

  while( 1 ) {
    util_sys_inb(STAT_REG, &stat); /*assuming it returns OK*/
    /*loop while 8042 input buffer is not empty*/
    if( (stat & IBF) == 0 ) {
      sys_outb(KBC_CMD_REG, READ_COMMAND); //sends read command to port 0x64
      break;
    }
    else return 1;
  }
  
  while( 1 ) {
    if (util_sys_inb(STAT_REG, &stat)) return 1; //gets status register value
    if( stat & OBF ) { //checks if output buffer is set
      if (((stat &(PAR_ERR | TO_ERR)) == 0)) { //Parity and Time Out errors check
        if(util_sys_inb(OUT_BUF, &command)) return 1; //reads command byte from port 0x60
        else break;
      } 
      else return 1;
    }
  }

  while( 1 ) {
    util_sys_inb(STAT_REG, &stat); /*assuming it returns OK*/
    /*loop while 8042 input buffer is not empty*/
    if( (stat & IBF) == 0 ) {
      sys_outb(KBC_CMD_REG, WRITE_COMMAND); //sends write command to port 0x64
      break;
    }
    else return 1;
  }

  command = command | EN_INT_OBF; //sets first and second bits of command byte to 1
  
  while( 1 ) {
    util_sys_inb(STAT_REG, &stat); /*assuming it returns OK*/
    /*loop while 8042 input buffer is not empty*/
    if( (stat & IBF) == 0 ) {
      sys_outb(OUT_BUF, command); //sends command to port 0x60
      break;
    }
    else return 1;
  }

  return 0;
}

