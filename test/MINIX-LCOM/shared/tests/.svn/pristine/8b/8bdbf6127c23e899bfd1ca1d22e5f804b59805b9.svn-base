#include <lcom/lcf.h>

#include <lcom/lab5.h>

#include <stdint.h>
#include <stdio.h>

#include <machine/int86.h>
#include <sys/mman.h>

#include "graphics.h"
#include "macro_graphics.h"

//global variables declaration
//static mmap_t *map;
static vbe_mode_info_t info;

//static mmap_t *video_mem;
static void* video_mem;		/* Process (virtual) address to which VRAM is mapped */

static unsigned h_res;	        /* Horizontal resolution in pixels */
static unsigned v_res;	        /* Vertical resolution in pixels */
static unsigned bits_per_pixel; /* Number of VRAM bits per pixel */

int get_mode_info(uint16_t mode, vbe_mode_info_t* info){
  reg86_t r;
  memset(&r, 0, sizeof(reg86_t));	/* zero the structure */

  mmap_t map;

  lm_alloc(sizeof(vbe_mode_info_t), &map); /*allocate memmory for map variable*/

  /*Fill the structure*/
  r.ah = BIOS_AH;
  r.al = SET_VBE_MODE;

  r.bx = LINEAR_FRAMEBUFFER | mode; // set bit 14: linear framebuffer

  r.es = PB2BASE(map.phys);
  r.di = PB2OFF(map.phys);

  r.intno = VBE_INTERFACE;

  if (sys_int86(&r) != OK) return 1;

  //memcpy(info, map.virt, map.size);
  *info = *(vbe_mode_info_t*) (map.virt);
  lm_free(&map); /* free previously allocated memmory*/

  return 0;
}

void * (vg_init)(uint16_t mode){

  reg86_t r;
  memset(&r, 0, sizeof(reg86_t));	/* zero the structure */

  int num;

  struct minix_mem_range mb;
  struct minix_mem_range mr;
  unsigned int vram_base;  /* VRAM's physical addresss */
  unsigned int vram_size;  /* VRAM's size, but you can use
              the frame-buffer size, instead */

  /*Set MB*/
  mb.mr_base = 0;
  mb.mr_limit = SIZE_MB;

  if( OK != (num = sys_privctl(SELF, SYS_PRIV_ADD_MEM, &mb)))
    panic("sys_privctl (ADD_MEM) failed: %d\n", num);
  
  
  /* Use VBE function 0x01 to initialize vram_base and vram_size */
  //if (vbe_get_mode_info(mode, &info)) return NULL; -> given funtion
  if (get_mode_info(mode, &info)) return NULL;

  vram_base = info.PhysBasePtr;
  h_res = info.XResolution;
  v_res = info.YResolution;
  bits_per_pixel = info.BitsPerPixel;

  unsigned int bytes_per_pixel = (bits_per_pixel + 7) / 8;
  vram_size = h_res * v_res * bytes_per_pixel; //size is expressed in bytes

  /* Allow memory mapping */

  mr.mr_base = (phys_bytes) vram_base;	
  mr.mr_limit = mr.mr_base + vram_size;  

  if( OK != (num = sys_privctl(SELF, SYS_PRIV_ADD_MEM, &mr)))
    panic("sys_privctl (ADD_MEM) failed: %d\n", num);

  /* Map memory */

  video_mem = vm_map_phys(SELF, (void *)mr.mr_base, vram_size);

  if(video_mem == MAP_FAILED)
    panic("couldn't map video memory");

  /*Set graphics mode*/

  r.ah = BIOS_AH;
  r.al = SET_VBE_MODE; //VBE set mode -- function 02
  r.bx = LINEAR_FRAMEBUFFER | mode; // set bit 14: linear framebuffer
  r.intno = VBE_INTERFACE;

  if( sys_int86(&r) != OK ) {
    printf("set_vbe_mode: sys_int86() failed \n");
    return NULL;
  }

  return video_mem;
}

int vg_color_pixel(uint16_t x, uint16_t y, uint32_t color){
  uint8_t *save = video_mem; // 1B

  unsigned int bytes_per_pixel = (bits_per_pixel + 7) / 8;

  //Place save in correct position
  save += h_res * y * bytes_per_pixel; //place in y coordinate
  save += x * bytes_per_pixel; //place in x coordinate

  //Go through each color byte
  for (unsigned int i = 0; i < bytes_per_pixel; i++){

    (*save) = color >> 8 * i; //save color byte
    save++;
  }
  return 0;
}


int (vg_draw_hline)(uint16_t x, uint16_t y, uint16_t len, uint32_t color){
  for (uint16_t i = 0; i < len; i++){
    if (vg_color_pixel((uint16_t)(x+i), y, color)) return 1;
  }

  return 0;
}

int (vg_draw_rectangle)(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color){

  for (uint16_t i = 0; i < height; i++){
    if ((vg_draw_hline)(x, y + i, width, color)) return 1;
  }

  return 0;
}


uint32_t generate_red(uint32_t first, uint8_t step, int col){
  //Get size and position of Red information
  int place = info.RedFieldPosition;
  int size = info.RedMaskSize;

  //Create red mask (puts numbers of bits equal to size as 1)
  uint32_t mask = 0;
  for (int i = 0; i < size; i++){
    mask |= BIT(i);
  }

  //Moves mask to correct place
  mask = mask << place;

  return ((((first & mask) >> place) + col * step) % (1 << size)) << place;
}

uint32_t generate_green(uint32_t first, uint8_t step, int row){
  //Get size and position of Green information
  int place = info.GreenFieldPosition;
  int size = info.GreenMaskSize;

  //Create green mask (puts numbers of bits equal to size as 1)
  uint32_t mask = 0;
  for (int i = 0; i < size; i++){
    mask |= BIT(i);
  }

  //Moves mask to correct place
  mask = mask << place;

  return ((((first & mask) >> place) + row * step) % (1 << size)) << place;
}

uint32_t generate_blue(uint32_t first, uint8_t step, int row, int col){
  //Get size and position of Blue information
  int place = info.BlueFieldPosition;
  int size = info.BlueMaskSize;

  //Create blue mask (puts numbers of bits equal to size as 1)
  uint32_t mask = 0;
  for (int i = 0; i < size; i++){
    mask |= BIT(i);
  }

  //Moves mask to correct place
  mask = mask << place;


  return ((((first & mask) >> place) + (row + col) * step) % (1 << size)) << place;
}


int vg_draw_line_of_rectangles(uint16_t x, uint16_t y, uint16_t mode, uint8_t no_rectangles, uint32_t first, uint8_t step, uint32_t color, uint16_t rec_x, uint16_t rec_y){
  //Get black stripe dimensions
  uint16_t bstripe_x = h_res % no_rectangles;

  for (int i = 0; i < no_rectangles; i++){
    //Set up left vertex of rectangle to draw coordinate
    x = rec_x * i;

    //Check index or direct color mode
    if (mode == INDEX_COLOR_MODE){
      color = (first + ((y/rec_y) * no_rectangles + i) * step) % (1 << bits_per_pixel);
    }
    else{
      color = generate_red(first, step, i) | generate_green(first, step, (y/rec_y)) | generate_blue(first, step, y/rec_y, i);
    }

    vg_draw_rectangle(x, y, rec_x, rec_y, color);
  }

  //Draw vertical black stripe
  x += rec_x;
  vg_draw_rectangle(x, y, bstripe_x, rec_y, BLACK_RGB);

  return 0;
}

int vg_draw_matrix(uint16_t x, uint16_t y, uint16_t mode, uint8_t no_rectangles, uint32_t first, uint8_t step, uint32_t color){
  //Get rectangle dimensions
  uint16_t rec_x = h_res / no_rectangles;
  uint16_t rec_y = v_res / no_rectangles;

  //Get black stripe dimensions
  uint16_t bstripe_y = v_res % no_rectangles;

  for (int i = 0; i < no_rectangles; i++){
    //Set height of line of rectangles
    y = rec_y * i;

    if (vg_draw_line_of_rectangles(x, y, mode, no_rectangles, first, step, color, rec_x, rec_y)) return 1;

    x = 0;
  }

  //Draw horizontal black stripe
  y += rec_y;
  vg_draw_rectangle(x, y, h_res, bstripe_y, BLACK_RGB);
  return 0;
}

int vg_draw_pattern(uint16_t mode, uint8_t no_rectangles, uint32_t first, uint8_t step){
  //Up left vertex coordinates of rectangle
  uint16_t x = 0, y = 0;
  uint32_t color = 0;

  //Draw matrix of rectangles
  if (vg_draw_matrix(x, y, mode, no_rectangles, first, step, color)) return 1;

  return 0;
}


int vg_draw_pixmap(xpm_map_t xpm, uint16_t x, uint16_t y){
  
  //Read pixmap
  enum xpm_image_type type = XPM_INDEXED;
  xpm_image_t img;
  uint8_t *sprite = xpm_load(xpm, type, &img);

  //Iterate over xpm height
  for (int row = 0; row < img.height; row++){
    //Iterate over xpm lenght
    for (int col = 0; col < img.width; col++){
      if (vg_color_pixel(col + x, row + y, sprite[row * img.width + col])) return 1;
    }
  }

  return 0;
}

int speed_handler(xpm_map_t xpm, uint16_t *xi, uint16_t *yi, uint16_t xf, uint16_t yf,
                     int16_t speed){
  //Increment top left corner coordinates
  //VERTICAL MOVEMENT
  if (*xi == xf) {
    if (speed > 0) {
      if (*yi + abs(speed) < yf) *yi += abs(speed);
      else if (*yi - abs(speed) > yf) *yi -= abs(speed);
      //Movement is smaller than speed
      else *yi = yf;
    }  
    else{
      if (*yi + 1 < yf) *yi += 1;
      else if (*yi - 1 > yf) *yi -= 1;
      //Movement is smaller than speed
      else *yi = yf;
    }  
  }    

  //HORIZONTAL MOVEMENT
  else {
    if (speed > 0){ 
      if (*xi + abs(speed) < xf) *xi += abs(speed);
      else if (*xi - abs(speed) > xf) *xi -= abs(speed);
      //Movement is smaller than speed
      else *xi = xf;
    }  
    else {
      if (*xi + 1 < xf) *xi += 1;
      else if (*xi - 1 > xf) *xi -= 1;
      //Movement is smaller than speed
      else *xi = xf;
    }
  }  

  /* Deletes track */
  uint8_t bytes_per_pixel = (bits_per_pixel + 7) / 8;
  memset(video_mem, 0, h_res*v_res*bytes_per_pixel);	

  //Draw pixmap in new coordinates
  if (vg_draw_pixmap(xpm, *xi, *yi)) return 1;

  return 0;

}

