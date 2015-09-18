#ifndef __ATCC_UI_H
#define __ATCC_UI_H 1

//#define MAX_PLANES 16
//#define MAX_AIRPORTS 10

#define MAP_WIDTH (LIST_CMDS+LIST_HEIGHT+BORDER)*2
#define MAP_HEIGHT LIST_CMDS+LIST_HEIGHT+BORDER
#define LIST_HEIGHT 20
#define LIST_WIDTH 20
#define LIST_CMDS 7
#define LOG_HEIGHT 1
#define CMD_HEIGHT 7

#define BORDER 2

#define PLANES_PER_PAGE 10

#define TIMEOUT 250

#define CMDS_NUM 6

void init();
void draw();
void dispose();
void clear_log();
void ui_test();	
void interpret_ch(int ch);
char *get_cmdname(int cmd);

#endif

