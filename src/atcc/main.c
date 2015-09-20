#include <ncurses.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include "../atcd/include/atcd.h"
#include "atcc.h"

static struct atc_state state;
static struct atc_ui ui;

int main(int argc, char ** argv)
{
	init_UI();
	while(1){
		update_state();
		draw_UI();
		input(wgetch(ui.cmd_log));
	}
	dispose_UI();
    return 0;
}

void update_state(void){
	state.planes_num = get_airplanes(state.planes);
	state.airports_num = get_airports(state.airports);
}

void input(int ch){
	if (ch >= '0' && ch <= '9'){
		if (ui.cur_plane == -1){
			if (ch-'0' < state.planes_num){
				ui.cur_plane = ch-'0';
				//clear_log();
				//struct atc_plane plane = planes[cur_page*PLANES_PER_PAGE+cur_plane];
				//mvwprintw(cmd_log, 0, 0, "%s %s", "Selected plane",plane.id);
			}
			return;
		}else if (ui.cur_cmd == -1){
			if (ch >= '0' && ch <= '6'){
				ui.cur_cmd = ch-'0';
				//clear_log();
				//mvwprintw(cmd_log, 0, 0, "%s %s", "Select command", get_cmdname(cur_cmd));
			}
			return;
		}
	}else if (ch == 66){
		if (ui.cur_page > 0){
			ui.cur_page--;
			clear_UI();
			mvwprintw(ui.cmd_log, 0, 0, "%s", "page down");
		}
	}else if (ch == 65){
		if (ui.cur_page+1 < state.planes_num/PLANES_PER_PAGE){
			ui.cur_page++;
			clear_UI();
			mvwprintw(ui.cmd_log, 0, 0, "%s", "page up");
		}
	}else if (ch == '\n' && ui.cur_cmd != -1 && ui.cur_plane != -1){
		struct atc_plane plane = state.planes[ui.cur_page*PLANES_PER_PAGE+ui.cur_plane];
		set(ui.cur_cmd, plane);
		clear_UI();
		if(ui.cur_cmd == speed_up){
			mvwprintw(ui.cmd_log, 0, 0, "%s %s", plane.id, "is speeding up");
		}else if(ui.cur_cmd == speed_down){
			mvwprintw(ui.cmd_log, 0, 0, "%s %s", plane.id, "is speeding down");
		}else if(ui.cur_cmd == climb){
			mvwprintw(ui.cmd_log, 0, 0, "%s %s", plane.id, "is ascending");
		}else if(ui.cur_cmd == descend){
			mvwprintw(ui.cmd_log, 0, 0, "%s %s", plane.id, "is descending");
		}else if(ui.cur_cmd == turn_right){
			mvwprintw(ui.cmd_log, 0, 0, "%s %s", plane.id, "is turning right");
		}else if(ui.cur_cmd == turn_left){
			mvwprintw(ui.cmd_log, 0, 0, "%s %s", plane.id, "is turning left");
		}
		ui.cur_plane = -1;
		ui.cur_cmd = -1;		
	}
}


void init_UI(void){
	initscr();
	noecho();
	keypad(ui.cmd_log, TRUE);
	nodelay(ui.cmd_log, FALSE);

	ui.brd_map = newwin(MAP_HEIGHT+BORDER, MAP_WIDTH+BORDER, 0, 0);
	ui.brd_plane_list = newwin(LIST_HEIGHT+BORDER, LIST_WIDTH+BORDER, 0, MAP_WIDTH+BORDER);
	ui.brd_cmds = newwin(LIST_CMDS+BORDER, LIST_WIDTH+BORDER, LIST_HEIGHT+BORDER, MAP_WIDTH+BORDER);
	ui.brd_cmd_log = newwin(LOG_HEIGHT+BORDER, MAP_WIDTH+LIST_WIDTH+BORDER*2, MAP_HEIGHT+BORDER, 0);

	ui.map = newwin(MAP_HEIGHT, MAP_WIDTH, 1, 1);
	ui.plane_list = newwin(LIST_HEIGHT, LIST_WIDTH, 1, MAP_WIDTH+BORDER+1);
	ui.cmds = newwin(LIST_CMDS, LIST_WIDTH, LIST_HEIGHT+BORDER+1, MAP_WIDTH+BORDER+1);
	ui.cmd_log = newwin(LOG_HEIGHT, MAP_WIDTH+LIST_WIDTH+BORDER, MAP_HEIGHT+BORDER+1, 1);
	
	box(ui.brd_map, 0, 0);
	box(ui.brd_cmd_log, 0, 0);
	box(ui.brd_plane_list, 0, 0);
	box(ui.brd_cmds, 0, 0);

	wrefresh(ui.brd_map);
	wrefresh(ui.brd_cmd_log);
	wrefresh(ui.brd_plane_list);
	wrefresh(ui.brd_cmds);
	
	ui.cur_page = 0;
	ui.cur_plane = -1;
	ui.cur_cmd = -1;
}

void draw_UI(void){
	wclear(ui.map);
	wclear(ui.plane_list);	
	wclear(ui.cmds);
	
	for (int i = 0; i < state.airports_num; i++){
		mvwprintw(ui.map, state.airports[i].y, state.airports[i].x, "@");
	}

	for (int i = 0; i < state.planes_num; i++){
		mvwprintw(ui.map, state.planes[i].y, state.planes[i].x, "+");
		if (i >= ui.cur_page*PLANES_PER_PAGE && i < PLANES_PER_PAGE*(ui.cur_page+1)){
			mvwprintw(ui.plane_list, i*2, 0, "%d: %s (%d, %d, %d)", i%PLANES_PER_PAGE, state.planes[i].id, state.planes[i].x, state.planes[i].y, state.planes[i].z);
			mvwchgat(ui.plane_list, i*2, 0, -1, A_BOLD, 0, NULL);
			if (ui.cur_plane >= 0 && i == ui.cur_page*PLANES_PER_PAGE+ui.cur_plane){
				mvwchgat(ui.plane_list, i*2, 0, -1, A_REVERSE, 0, NULL);
			}
			switch (state.planes[i].status){
				case landed:
					mvwprintw(ui.plane_list, i*2+1, 0, "Landed");
					break;
				case crashed:
					mvwprintw(ui.plane_list, i*2+1, 0, "Crashed");
					break;
				case flying:
					mvwprintw(ui.plane_list, i*2+1, 0, "Flying");
					break;
			}
		}		
	}

	for (int i = 0; i < CMDS_NUM; i++){
		mvwprintw(ui.cmds, i, 0, "%d: %s", i, get_cmdname(i));
		if (ui.cur_cmd != -1){
			mvwchgat(ui.cmds, ui.cur_cmd, 0, -1, A_REVERSE, 0, NULL);
		}
	}

	wrefresh(ui.map);
	wrefresh(ui.cmds);
	wrefresh(ui.plane_list);
	wrefresh(ui.cmd_log);
}

void clear_UI(void){
	wmove(ui.cmd_log, 0, 0);
	wclrtoeol(ui.cmd_log);
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

void dispose_UI(void){
	delwin(ui.map);
	delwin(ui.cmd_log);
	delwin(ui.plane_list);
	endwin();
}
