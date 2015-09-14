#include <ncurses.h>
#include <stdio.h>
#include <unistd.h>
#include "./include/ui.h"
#include "../atcd/include/atcd.h"

static WINDOW *map, *cmd_log, *plane_list, *actions;
static WINDOW *brd_map, *brd_cmd_log, *brd_plane_list, *brd_actions;
static int cur_line;
static struct atc_plane planes[MAX_PLANES];

void ui_test(){
	init();
	while(1){
		draw();
		//cmd_scan();
	}
	dispose();
}


void cmd_scan(){
	char scan_cmd[32];
	char scan_plane[32];
	wscanw(cmd, "%s:%s", &scan_cmd, &scan_plane);
	execute_cmd(&scan_cmd, &scan_plane);	
	wclear(cmd);
}

void execute_cmd(char *scan_cmd, char *scan_plane){
	struct atc_plane plane;
	int found_plane = get_plane(scan_plane, &plane);
	if (!found_plane){
		print_cmd("Plane doesnt exist");
	}
	if(strcmp(scan_cmd, "SPDUP")){
		set(speed_up, plane);
	}else if(strcmp(scan_cmd, "SPDDW")){
		set(speed_down, plane);
	}else if(strcmp(scan_cmd, "UP")){
		set(climb, plane);
	}else if(strcmp(scan_cmd, "DN")){
		set(descend, plane);
	}else if(strcmp(scan_cmd, "TR")){
		set(turn_right, plane);
	}else if(strcmp(scan_cmd, "TL")){
		set(turn_left, plane);
	}else{
		print_cmd("Unsupported command");
	}
}

int get_plane(char *plane_name, struct atc_plane *plane){
	int i;
	for (i = 0; i < planes_num; i++){
		if (strcmp(plane_name, planes[i].id)){
			plane = planes[i];
			return 1;
		}
	}
	return 0;
}

void init(){
	initscr();
	init_brd();

	map = newwin(MAP_HEIGHT, MAP_WIDTH, 1, 1);
	plane_list = newwin(MAP_HEIGHT, LIST_WIDTH, 1, MAP_WIDTH+BORDER*2+1);
	cmd_log = newwin(LOG_HEIGHT, MAP_WIDTH, MAP_HEIGHT+BORDER*2+1, 1);
	actions = newwin(LOG_HEIGHT, LIST_WIDTH, MAP_HEIGHT+BORDER*2+1, MAP_WIDTH+BORDER*2+1);
	cur_line = 0;	
}

void init_brd(){
	brd_map = newwin(MAP_HEIGHT, MAP_WIDTH, 0, 0);
	brd_plane_list = newwin(MAP_HEIGHT, LIST_WIDTH, 0, MAP_WIDTH+BORDER*2);
	brd_cmd_log = newwin(LOG_HEIGHT, MAP_WIDTH, MAP_HEIGHT+BORDER*2, 0);
	brd_actions = newwin(LOG_HEIGHT, LIST_WIDTH, MAP_HEIGHT+BORDER*2, MAP_WIDTH+BORDER*2);

	box(brd_map, 0, 0);
	box(brd_cmd_log, 0, 0);
	box(brd_plane_list, 0, 0);
	box(brd_actions, 0, 0);

	wrefresh(brd_map);
	wrefresh(brd_cmd_log);
	wrefresh(brd_plane_list);
	wrefresh(brd_actions);
}

void draw(){
	wclear(map);
	wclear(plane_list);	

	// //!!!CHANGE TO ACTUAL FUNCTION!!!
	planes = {{"AA1234", 1, 2, 3}, {"XX1234", 5, 3, 1,}};
	int planes_num = 2;
	planes[0].status = landed;
	planes[1].status = crashed;

	//FUNCION POSTA
	//struct atc_plane planes[MAX_PLANES];
	//int planes_num = get_planes(planes);

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

		mvwprintw(plane_list, i*2, 0, "%s (%d, %d, %d)", planes[i].id, planes[i].x, planes[i].y, planes[i].z);
		mvwchgat(plane_list, i*2, 0, -1, A_BOLD, 0, NULL);
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

	wrefresh(map);
	wrefresh(plane_list);
}

void print_cmd(char *str){
	wmove(cmd_log, cur_line, 0);
	wclrtoeol(cmd_log);
	mvwprintw(cmd_log, cur_line++, 0, str);
	cur_line %= LOG_HEIGHT;
	wrefresh(cmd_log);
}

void dispose(){
	delwin(map);
	delwin(cmd_log);
	delwin(plane_list);
	delwin(cmd);
	endwin();
}