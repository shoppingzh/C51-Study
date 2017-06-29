#include <reg51.h>
#include "lcd.c"

extern void write_com(uchar com);
extern void write_data(uchar datas);
extern void write_data_pos(uint pos,uchar datas);
extern void write_string(uchar *str);
extern uchar read_data();
extern uchar read_data_pos(uint pos);
extern uint read_pointer();
extern void display_init();