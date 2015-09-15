#ifndef __ATCC_UI_H
#define __ATCC_UI_H 1

#define MAX_PLANES 16
#define MAX_AIRPORTS 10

#define MAP_WIDTH 50
#define MAP_HEIGHT 25
#define LIST_WIDTH 20
#define LOG_HEIGHT 1
#define CMD_HEIGHT 1

#define BORDER 2

#define PLANES_PER_PAGE 10

#define TIMEOUT 250

void init();
void draw();
void dispose();
void clear_log();
void ui_test();	
void interpret_ch(int ch);

#endif

