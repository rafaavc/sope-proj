#include <lcom/lcf.h>

#include <lcom/lab5.h>

#include <stdint.h>
#include <stdio.h>


int set_vbe_mode(uint16_t mode);

int vg_color_pixel(uint16_t x, uint16_t y, uint32_t color);

int vg_draw_line_of_rectangles(uint16_t x, uint16_t y, uint16_t mode, uint8_t no_rectangles, uint32_t first, uint8_t step, uint32_t color, uint16_t rec_x, uint16_t rec_y);

int vg_draw_matrix(uint16_t x, uint16_t y, uint16_t mode, uint8_t no_rectangles, uint32_t first, uint8_t step, uint32_t color);

int vg_draw_pattern(uint16_t mode, uint8_t no_rectangles, uint32_t first, uint8_t step);

uint32_t generate_red(uint32_t first, uint8_t step, int col);
uint32_t generate_green(uint32_t first, uint8_t step, int row);
uint32_t generate_blue(uint32_t first, uint8_t step, int row, int col);

int vg_draw_pixmap(xpm_map_t xpm, uint16_t x, uint16_t y);

int positive_horizontal(const char *xpm[], uint16_t xi, uint16_t yi, uint16_t xf, uint16_t yf, int16_t speed, uint8_t fr_rate);
int negative_horizontal(const char *xpm[], uint16_t xi, uint16_t yi, uint16_t xf, uint16_t yf, int16_t speed, uint8_t fr_rate);

int positive_vertical(const char *xpm[], uint16_t xi, uint16_t yi, uint16_t xf, uint16_t yf, int16_t speed, uint8_t fr_rate);
int negative_vertical(const char *xpm[], uint16_t xi, uint16_t yi, uint16_t xf, uint16_t yf, int16_t speed, uint8_t fr_rate);


typedef struct {
  int x, y; // current position
  int width, height;  // dimensions
  int xspeed, yspeed; // current speed
  char*map;          // the pixmap
} Sprite;

int speed_handler(xpm_map_t xpm, uint16_t *xi, uint16_t *yi, uint16_t xf, uint16_t yf,
                     int16_t speed);

int get_mode_info(uint16_t mode, vbe_mode_info_t* info);
