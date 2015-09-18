#include <ncurses.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include "./include/ui.h"
#include "../atcd/include/atcd.h"

static WINDOW *map, *cmd_log, *plane_list, *cmds;
static WINDOW *brd_map, *brd_cmd_log, *brd_plane_list, *brd_cmds;
static int cur_line;

static struct atc_plane planes[MAX_PLANES] = {{"AA1234", 1, 2, 3}, {"XX1234", 5, 3, 1,}};
static int planes_num;

static int cur_plane;
static int cur_cmd;
static int cur_page;

void ui_test(void){
	init();
	while(1){
		draw();
		interpret_ch(wgetch(cmd_log));
	}
	dispose();
}

void interpret_ch(int ch){
	if (ch >= '0' && ch <= '9'){
		if (cur_plane == -1){
			if (ch-'0' < planes_num){
				cur_plane = ch-'0';
				//clear_log();
				//struct atc_plane plane = planes[cur_page*PLANES_PER_PAGE+cur_plane];
				//mvwprintw(cmd_log, 0, 0, "%s %s", "Selected plane",plane.id);
			}
			return;
		}else if (cur_cmd == -1){
			if (ch >= '0' && ch <= '6'){
				cur_cmd = ch-'0';
				//clear_log();
				//mvwprintw(cmd_log, 0, 0, "%s %s", "Select command", get_cmdname(cur_cmd));
			}
			return;
		}
	}else if (ch == 66){
		if (cur_page > 0){
			cur_page--;
			clear_log();
			mvwprintw(cmd_log, cur_line, 0, "%s", "page down");
		}
	}else if (ch == 65){
		if (cur_page+1 < planes_num/PLANES_PER_PAGE){
			cur_page++;
			clear_log();
			mvwprintw(cmd_log, cur_line, 0, "%s", "page up");
		}
	}else if (ch == '\n' && cur_cmd != -1 && cur_plane != -1){
		struct atc_plane plane = planes[cur_page*PLANES_PER_PAGE+cur_plane];
		//set(cur_cmd, plane);
		clear_log();
		if(cur_cmd == speed_up){
			mvwprintw(cmd_log, cur_line, 0, "%s %s", plane.id, "is speeding up");
		}else if(cur_cmd == speed_down){
			mvwprintw(cmd_log, cur_line, 0, "%s %s", plane.id, "is speeding down");
		}else if(cur_cmd == climb){
			mvwprintw(cmd_log, cur_line, 0, "%s %s", plane.id, "is ascending");
		}else if(cur_cmd == descend){
			mvwprintw(cmd_log, cur_line, 0, "%s %s", plane.id, "is descending");
		}else if(cur_cmd == turn_right){
			mvwprintw(cmd_log, cur_line, 0, "%s %s", plane.id, "is turning right");
		}else if(cur_cmd == turn_left){
			mvwprintw(cmd_log, cur_line, 0, "%s %s", plane.id, "is turning left");
		}
		cur_plane = -1;
		cur_cmd = -1;		
	}
}


void init(void){
	initscr();
	noecho();
	keypad(cmd_log, TRUE);
	nodelay(cmd_log, FALSE);

	brd_map = newwin(MAP_HEIGHT+BORDER, MAP_WIDTH+BORDER, 0, 0);
	brd_plane_list = newwin(LIST_HEIGHT+BORDER, LIST_WIDTH+BORDER, 0, MAP_WIDTH+BORDER);
	brd_cmds = newwin(CMD_HEIGHT+BORDER, LIST_WIDTH+BORDER, LIST_HEIGHT+BORDER, MAP_WIDTH+BORDER);
	brd_cmd_log = newwin(LOG_HEIGHT+BORDER, MAP_WIDTH+LIST_WIDTH+BORDER*2, MAP_HEIGHT+BORDER, 0);

	map = newwin(MAP_HEIGHT, MAP_WIDTH, 1, 1);
	plane_list = newwin(LIST_HEIGHT, LIST_WIDTH, 1, MAP_WIDTH+BORDER+1);
	cmds = newwin(CMD_HEIGHT, LIST_WIDTH, LIST_HEIGHT+BORDER+1, MAP_WIDTH+BORDER+1);
	cmd_log = newwin(LOG_HEIGHT, MAP_WIDTH+LIST_WIDTH+BORDER, MAP_HEIGHT+BORDER+1, 1);
	
	box(brd_map, 0, 0);
	box(brd_cmd_log, 0, 0);
	box(brd_plane_list, 0, 0);
	box(brd_cmds, 0, 0);

	wrefresh(brd_map);
	wrefresh(brd_cmd_log);
	wrefresh(brd_plane_list);
	wrefresh(brd_cmds);

	cur_line = 0;	
	cur_page = 0;
	cur_plane = -1;
	cur_cmd = -1;
}

void draw(void){
	wclear(map);
	wclear(plane_list);	
	wclear(cmds);	

	// //!!!CHANGE TO ACTUAL FUNCTION!!!
	planes_num = 2;
	planes[0].status = landed;
	planes[1].status = crashed;

	//FUNCION POSTA
	//planes_num = get_planes(planes);

	// //!!!CHANGE FOR ACTUAL FUNCTION!!!
	struct atc_airport airports[MAX_AIRPORTS] = {{13, 15, "LAX"}, {10, 10, "EZE"}};
	int airports_num = 2;

	//FUNCION POSTA
	//struct atc_airport airports[MAX_AIRPORTS];
	//int airports_num = get_airports(airports);
	
	int i;
	for (i = 0; i < airports_num; i++){
		mvwprintw(map, airports[i].y, airports[i].x, "@");
		mvwprintw(map, airports[i].y+1, airports[i].x, "@");
		mvwprintw(map, airports[i].y, airports[i].x+1, "@");
		mvwprintw(map, airports[i].y+1, airports[i].x+1, "@");
	}

	for (i = 0; i < planes_num; i++){
		mvwprintw(map, planes[i].y, planes[i].x, "+");
		if (i >= cur_page*PLANES_PER_PAGE && i < PLANES_PER_PAGE*(cur_page+1)){
			mvwprintw(plane_list, i*2, 0, "%d: %s (%d, %d, %d)", i%PLANES_PER_PAGE, planes[i].id, planes[i].x, planes[i].y, planes[i].z);
			mvwchgat(plane_list, i*2, 0, -1, A_BOLD, 0, NULL);
			if (cur_plane >= 0 && i == cur_page*PLANES_PER_PAGE+cur_plane){
				mvwchgat(plane_list, i*2, 0, -1, A_REVERSE, 0, NULL);
			}
			switch (planes[i].status){
				case landed:
					mvwprintw(plane_list, i*2+1, 0, "Landed");
					break;
				case crashed:
					mvwprintw(plane_list, i*2+1, 0, "Crashed");
					break;
				case flying:
					mvwprintw(plane_list, i*2+1, 0, "Flying");
					break;
			}
		}		
	}

	for (i = 0; i < CMDS_NUM; i++){
		mvwprintw(cmds, i, 0, "%d: %s", i, get_cmdname(i));
		if (cur_cmd != -1){
			mvwchgat(cmds, cur_cmd, 0, -1, A_REVERSE, 0, NULL);
		}
	}

	wrefresh(map);
	wrefresh(cmds);
	wrefresh(plane_list);
	wrefresh(cmd_log);
}

void clear_log(void){
	wmove(cmd_log, 0, 0);
	wclrtoeol(cmd_log);
}

char *get_cmdname(int cmd){
	switch (cmd){
		case 0:
			return "Speed up";
		case 1:	
			return "Speed down";
		case 2:
			return "Ascend";
		case 3:
			return "Descend";
		case 4:
			return "Turn right";
		default:
			return "Turn left";
	}
}

void dispose(void){
	delwin(map);
	delwin(cmd_log);
	delwin(plane_list);
	endwin();
}

