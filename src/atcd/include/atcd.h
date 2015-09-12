#ifndef __ATCD_H
#define __ATCD_H 1

#include <time.h>

enum atc_heading { SW, W, NW, N, NE, E, SE, S };
enum atc_status{landed, crashed, flying};
enum atc_elevation { UP_1 = 10, UP_2 = 20, UP_3 = 30, STRAIGTH = 0, DOWN_1 = -10, DOWN_2 = -20, DOWN_3 = -30 };


struct atc_plane { 
    char id[6];
    int x;
    int y;
    int z;
    time_t time;
    enum atc_heading heading;
    enum atc_elevation elevation;
    char speed;
    enum atc_status status;
};

struct atc_airport{
	int x;
	int y;
	char id[3];
};

enum atc_commands{speed_up, speed_down, climb, descend, turn_rigth, turn_left};

int atcd_test(void);

void atc_new_game();

struct atc_plane new_plane();

time_t get_time();

void blip();

int get_airports(struct atc_airport ports[]);


/*
Position calculation

Tc-Ts = Diff

h*s*diff + (x|y)
e*s*diff + z


void calculate_position(atc_plane* plane, time_t new_time){
	time_t dif_time = new_time - plane->time;
	plane->x = 


	
}

*/




#endif

