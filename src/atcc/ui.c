#include <ncurses.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include "./include/ui.h"
#include "../atcd/include/atcd.h"

static WINDOW *map, *cmd_log, *plane_list, *actions;
static WINDOW *brd_map, *brd_cmd_log, *brd_plane_list, *brd_actions;
static int cur_line;

static struct atc_plane planes[MAX_PLANES] = {{"AA1234", 1, 2, 3}, {"XX1234", 5, 3, 1,}};
static int planes_num;

static int cur_plane;
static int cur_cmd;
static int cur_page;

void ui_test(){
	init();
	while(1){
		draw();
		print_cmd("asd");
		int ch = getch();
		interpret_ch(ch);
	}
	dispose();
}

void interpret_ch(int ch){
	print_cmd("HOLA K ASE");
	if (ch == ERR){
		return;
	}else if (ch >= 0 && ch < 10){
		if (cur_plane == -1){
			if (ch < planes_num){
				cur_plane = ch;
			}
			return;
		}else if (cur_cmd == -1){
			if (ch >= 0 && ch < 6){
				cur_cmd = ch;
			}
			return;
		}
	}else if (ch == KEY_DOWN){
		if (cur_page > 0){
			cur_page--;
		}
	}else if (ch == KEY_UP){
		if (cur_page < MAX_PLANES/PLANES_PER_PAGE){
			cur_page++;
		}
	}else if (ch == KEY_BREAK && cur_cmd >= 0 && cur_plane >= 0){
		//struct atc_plane plane = planes[cur_page*PLANES_PER_PAGE+cur_plane];
		//set(cur_cmd, plane);
		if(cur_cmd == speed_up){
			print_cmd("Speeding up");
		}else if(cur_cmd == speed_down){
			print_cmd("Speeding down");
		}else if(cur_cmd == climb){
			print_cmd("Ascending");
		}else if(cur_cmd == descend){
			print_cmd("Descending");
		}else if(cur_cmd == turn_right){
			print_cmd("Turning right");
		}else if(cur_cmd == turn_left){
			print_cmd("Turning left");
		}
		cur_plane = -1;
		cur_cmd = -1;
	}
}

void init(){
	initscr();
	cbreak();
	noecho();
	timeout(TIMEOUT);

	brd_map = newwin(MAP_HEIGHT+BORDER, MAP_WIDTH+BORDER, 0, 0);
	brd_plane_list = newwin(MAP_HEIGHT+BORDER, LIST_WIDTH+BORDER, 0, MAP_WIDTH+BORDER);
	brd_cmd_log = newwin(ACTION_HEIGHT+BORDER, MAP_WIDTH+BORDER, MAP_HEIGHT+BORDER, 0);
	brd_actions = newwin(ACTION_HEIGHT+BORDER, LIST_WIDTH+BORDER, MAP_HEIGHT+BORDER, MAP_WIDTH+BORDER);

	map = newwin(MAP_HEIGHT, MAP_WIDTH, 1, 1);
	plane_list = newwin(MAP_HEIGHT, LIST_WIDTH, 1, MAP_WIDTH+BORDER+1);
	cmd_log = newwin(ACTION_HEIGHT, MAP_WIDTH, MAP_HEIGHT+BORDER+1, 1);
	actions = newwin(ACTION_HEIGHT, LIST_WIDTH, MAP_HEIGHT+BORDER+1, MAP_WIDTH+BORDER+1);
	
	cur_line = 0;	
	cur_page = 0;
	cur_plane = -1;
	cur_cmd = -1;
}

void draw(){
	wclear(map);
	wclear(plane_list);	

	box(brd_map, 0, 0);
	box(brd_cmd_log, 0, 0);
	box(brd_plane_list, 0, 0);
	box(brd_actions, 0, 0);

	wrefresh(brd_map);
	wrefresh(brd_cmd_log);
	wrefresh(brd_plane_list);
	wrefresh(brd_actions);
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
			mvwprintw(plane_list, i*2, 0, "%s (%d, %d, %d)", planes[i].id, planes[i].x, planes[i].y, planes[i].z);
			mvwchgat(plane_list, i*2, 0, -1, A_BOLD, 0, NULL);
			if (cur_plane > 0 && i == cur_page*PLANES_PER_PAGE+cur_plane){
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

	wrefresh(map);
	wrefresh(plane_list);
}

void print_cmd(char *str){
	wmove(cmd_log, cur_line, 0);
	wclrtoeol(cmd_log);
	mvwprintw(cmd_log, cur_line++, 0, str);
	cur_line %= ACTION_HEIGHT;
	wrefresh(cmd_log);
}

void dispose(){
	delwin(map);
	delwin(cmd_log);
	delwin(plane_list);
	delwin(actions);
	endwin();
}