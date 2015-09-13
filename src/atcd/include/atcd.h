#ifndef __ATCD_H
#define __ATCD_H 1

#include <time.h>
#include <math.h>
#include <time.h>
#include <storage.h>

#define LANDING_TOLERANCE 200
#define MAX_LEN 40
#define MAX_HEIGHT 20
#define MAX_AIRPORTS 2


//ENUMS & STRUCTS

enum atc_heading { SW=225, W=180, NW=135, N=90, NE=45, E=0, SE=315, S=270 };
enum atc_status{landed, crashed, flying};
enum atc_elevation { UP_1 = 10, UP_2 = 20, UP_3 = 30, STRAIGTH = 0, DOWN_1 = -10, DOWN_2 = -20, DOWN_3 = -30 };
enum atc_commands{speed_up, speed_down, climb, descend, turn_rigth, turn_left};


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



//HEADERS

void calculate_position(struct atc_plane* , time_t);

int atcd_test(void);

void atc_new_game();

struct atc_plane new_plane();

time_t get_time();

void blip();

int get_airports(struct atc_airport ports[]);

time_t get_time();

int set(enum atc_commands cmd, struct atc_plane plane);

int get_airplanes();



#endif

