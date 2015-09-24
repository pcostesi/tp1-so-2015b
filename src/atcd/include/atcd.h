#ifndef __ATCD_H
#define __ATCD_H 1

#include <time.h>
#include <math.h>
#include <time.h>
#include <storage.h>

#define LANDING_TOLERANCE 200
#define MAX_LEN 4000
#define MAX_HEIGHT 4000
#define MAX_AIRPORTS 2
#define MAX_PLANES 50
#define ATCD_ID_LENGTH 6

enum atc_heading { SW=225, W=180, NW=135, N=90, NE=45, E=0, SE=315, S=270 };
enum atc_status{landed, crashed, flying};
enum atc_elevation { UP_1 = 10, UP_2 = 20, UP_3 = 30, STRAIGTH = 0, DOWN_1 = -10, DOWN_2 = -20, DOWN_3 = -30 };
enum atc_commands{speed_up, speed_down, climb, descend, turn_right, turn_left};


struct atc_plane { 
    char id[ATCD_ID_LENGTH];
    int x; /*expressed in mts*/
    int y; /*expressed in mts*/
    int z;/*expressed in mts*/
    time_t time; 
    enum atc_heading heading; /*expressed in cardinal points*/
    enum atc_elevation elevation; /*expressed in elevation angle*/
    int speed; /*expressed in m/s*/
    enum atc_status status;
};

struct atc_airport{
	int x;
	int y;
	char id[3];
};

int calculate_position(struct atc_plane* , time_t);

int atcd_test(void);

void atc_new_game(void);

time_t get_time(void);

void blip(void);

int get_airports(struct atc_airport ports[]);

time_t get_time(void);

int set(enum atc_commands cmd, struct atc_plane *plane);

int get_airplanes(struct atc_plane buffer[]);

int atc_init(void);

void create_plane(void);

#endif

