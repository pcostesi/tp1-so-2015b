#ifndef __ATCC_UI_H
#define __ATCC_UI_H 1

#define MAX_PLANES 16
#define MAX_AIRPORTS 10

#define MAP_WIDTH 20
#define MAP_HEIGHT 20
#define LIST_WIDTH 20
#define LOG_HEIGHT 4
#define CMD_HEIGHT 1

#define BORDER 2

void init();
void init_brd();
void draw();
void dispose();
void print_cmd(char *str);
void ui_test();	
void cmd_scan();

#endif

