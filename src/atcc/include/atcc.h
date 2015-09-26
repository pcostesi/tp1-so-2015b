#ifndef __ATCC_H
#define __ATCC_H 1

#define LIST_HEIGHT 20
#define LIST_WIDTH 30
#define LIST_CMDS 6
#define LOG_HEIGHT 1
#define BORDER 2
#define MAP_HEIGHT LIST_CMDS+LIST_HEIGHT+BORDER
#define MAP_WIDTH (MAP_HEIGHT)*2

#define PLANES_PER_PAGE 10

#define CMDS_NUM 6

struct atc_state{
	struct atc_plane planes[MAX_PLANES];
	struct atc_airport airports[MAX_AIRPORTS];
	int planes_num;
	int airports_num;
	time_t cur_time;
	time_t tick_time;
	int crashed_planes;
	int landed_planes;
};

struct atc_ui{
	WINDOW *map, *cmd_log, *plane_list, *cmds, *score;
	WINDOW *brd_map, *brd_cmd_log, *brd_plane_list, *brd_cmds, *brd_score;

	int cur_plane;
	int cur_cmd;
	int cur_page;
};

void init_UI();
void draw_UI();
void dispose_UI();
void clear_UI();
void input(int ch);
char *get_cmdname(int cmd);
char *get_status(enum atc_status status);

void join_game(void);
void leave_game(void);
void update_state(void);
void init_time(void);

#endif
