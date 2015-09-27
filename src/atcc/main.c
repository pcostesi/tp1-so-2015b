#include <ncurses.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include "atcd.h"
#include "atcc.h"
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static struct atc_state state;
static struct atc_ui ui;

void init_signal_handler(void);
void cli_sigint_handler(int sig);


int main(int argc, char ** argv)
{
	init_signal_handler();
	atc_init();
	init_UI();
	init_time();
	create_plane();
	state.airports_num = get_airports(state.airports);
	while(1){
		update_state();
		draw_UI();
		input(wgetch(ui.cmd_log));
	}
	atc_deinit();
	dispose_UI();
    return 0;
}


void init_signal_handler(void){
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = cli_sigint_handler;
    sigaction(SIGINT, &sa, NULL);
}

void cli_sigint_handler(int sig){
	atc_deinit();
	dispose_UI();
	exit(0);
}


void init_time(void){
	state.tick_time = time(NULL);
}

void update_state(void){
	state.planes_num = get_airplanes(state.planes);
	state.crashed_planes = get_crashed();
	state.landed_planes = get_landed();
	state.cur_time = time(NULL);
	if (difftime(state.cur_time, state.tick_time) > 20){
		state.tick_time = state.cur_time;
		create_plane();
	}
}

void input(int ch){
	if (ch >= '0' && ch <= '9'){
		if (ui.cur_plane == -1){
			if (ch-'0' < state.planes_num){
				ui.cur_plane = ch-'0';
			}
			return;
		}else if (ui.cur_cmd == -1){
			if (ch >= '0' && ch <= '6'){
				ui.cur_cmd = ch-'0';
			}
			return;
		}
	}else if (ch == 66){
		if (ui.cur_page > 0){
			ui.cur_page--;
			clear_UI();
			mvwprintw(ui.cmd_log, 0, 0, "Moved to page %d", ui.cur_page+1);
		}
	}else if (ch == 65){
		if (ui.cur_page+1 < state.planes_num/(float)PLANES_PER_PAGE){
			ui.cur_page++;
			clear_UI();
			mvwprintw(ui.cmd_log, 0, 0, "Moved to page %d", ui.cur_page+1);
		}
	}else if (ch == '\n' && ui.cur_cmd != -1 && ui.cur_plane != -1){
		struct atc_plane plane = state.planes[ui.cur_page*PLANES_PER_PAGE+ui.cur_plane];
		set(ui.cur_cmd, &plane);
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
	halfdelay(1);

	ui.brd_map = newwin(MAP_HEIGHT+BORDER, MAP_WIDTH+BORDER, 0, 0);
	ui.brd_plane_list = newwin(LIST_HEIGHT+BORDER, LIST_WIDTH+BORDER, 0, MAP_WIDTH+BORDER);
	ui.brd_cmds = newwin(LIST_CMDS+BORDER, LIST_WIDTH+BORDER, LIST_HEIGHT+BORDER, MAP_WIDTH+BORDER);
	ui.brd_cmd_log = newwin(LOG_HEIGHT+BORDER, MAP_WIDTH+BORDER, MAP_HEIGHT+BORDER, 0);
	ui.brd_score = newwin(LOG_HEIGHT+BORDER, LIST_WIDTH+BORDER, MAP_HEIGHT+BORDER, MAP_WIDTH+BORDER);

	ui.map = newwin(MAP_HEIGHT, MAP_WIDTH, 1, 1);
	ui.plane_list = newwin(LIST_HEIGHT, LIST_WIDTH, 1, MAP_WIDTH+BORDER+1);
	ui.cmds = newwin(LIST_CMDS, LIST_WIDTH, LIST_HEIGHT+BORDER+1, MAP_WIDTH+BORDER+1);
	ui.cmd_log = newwin(LOG_HEIGHT, MAP_WIDTH, MAP_HEIGHT+BORDER+1, 1);
	ui.score = newwin(LOG_HEIGHT, LIST_WIDTH, MAP_HEIGHT+BORDER+1, MAP_WIDTH+BORDER+1);
	
	box(ui.brd_map, 0, 0);
	box(ui.brd_cmd_log, 0, 0);
	box(ui.brd_plane_list, 0, 0);
	box(ui.brd_cmds, 0, 0);
	box(ui.brd_score, 0, 0);

	wrefresh(ui.brd_map);
	wrefresh(ui.brd_cmd_log);
	wrefresh(ui.brd_plane_list);
	wrefresh(ui.brd_cmds);
	wrefresh(ui.brd_score);
	
	ui.cur_page = 0;
	ui.cur_plane = -1;
	ui.cur_cmd = -1;
}

void draw_UI(void){
	int i;

	wclear(ui.plane_list);	
	wclear(ui.cmds);
	wclear(ui.map);
	wclear(ui.score);
	
	for (i = 0; i < state.airports_num; i++){
		mvwprintw(ui.map, (state.airports[i].y/(float)MAX_HEIGHT)*(MAP_HEIGHT), (state.airports[i].x/(float)MAX_LEN)*(MAP_WIDTH), "@");
	}

	for (i = 0; i < state.planes_num; i++){
		mvwprintw(ui.map, (state.planes[i].y/(float)MAX_HEIGHT)*(MAP_HEIGHT), (state.planes[i].x/(float)MAX_LEN)*(MAP_WIDTH), "%d", i);
		if (i >= ui.cur_page*PLANES_PER_PAGE && i < PLANES_PER_PAGE*(ui.cur_page+1)){
			mvwprintw(ui.plane_list, (i%PLANES_PER_PAGE)*2, 0, "%d: %s (%d, %d, %d)", i, state.planes[i].id, state.planes[i].x, state.planes[i].y, state.planes[i].z);
			mvwchgat(ui.plane_list, (i%PLANES_PER_PAGE)*2, 0, -1, A_BOLD, 0, NULL);
			if (ui.cur_plane >= 0 && i == ui.cur_page*PLANES_PER_PAGE+ui.cur_plane){
				mvwchgat(ui.plane_list, (i%PLANES_PER_PAGE)*2, 0, -1, A_REVERSE, 0, NULL);
			}
			mvwprintw(ui.plane_list, (i%PLANES_PER_PAGE)*2+1, 0, "%s E:%d S:%d H:%d", get_status(state.planes[i].status), state.planes[i].elevation, (state.planes[i].speed/14)*50, state.planes[i].heading);
		}		
	}

	for (i = 0; i < CMDS_NUM; i++){
		mvwprintw(ui.cmds, i, 0, "%d: %s", i, get_cmdname(i));
		if (ui.cur_cmd != -1){
			mvwchgat(ui.cmds, ui.cur_cmd, 0, -1, A_REVERSE, 0, NULL);
		}
	}

	mvwprintw(ui.score, 0, 0, "Landed:%d Crashed:%d", state.landed_planes, state.crashed_planes);

	wrefresh(ui.map);
	wrefresh(ui.cmds);
	wrefresh(ui.plane_list);	
	wrefresh(ui.score);
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

char *get_status(enum atc_status status){
	switch (status){
		case landed:
			return "Landed";
		case crashed:	
			return "Crashed";
		default:
			return "Flying";
	}
}

void dispose_UI(void){
	delwin(ui.map);
	delwin(ui.cmd_log);
	delwin(ui.plane_list);
	endwin();
}
