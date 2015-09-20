#include "atcd.h"
#include <string.h>
#include <math.h>
#include <stdlib.h>

int atcd_test(void)
{
    return 5;
}

//Static Headers

static void _set_plane_x_y(struct atc_plane *plane, int time_dif);
static void _crashed_or_landed(struct atc_plane *plane);
static int _in_da_zone(struct atc_plane *plane);
static double _sin(int angle);
static float _cos(int angle);
static int _rand_number();
static int _rand_capital_letter();
static int _flying_planes(struct atc_plane plane, char name[6]);

static struct sto_database sto_db;
static int _planes_count = 0;

//esto va en main sto_init(sto_db, "Planes", sizeof(struct atc_plane));

//Returns 0 if plane crashed or landed, else returns 1, also calculates plane position and modifies it on the plane tuple
int calculate_position(struct atc_plane* plane, time_t new_time)
{
	int time_dif = new_time - plane->time;
	int aux = (int)(_sin(plane->elevation) * plane->speed * time_dif + plane->z ) ;
	if( aux < 0 ){
		int time_of_crash = new_time - plane->time - ( (aux*(-1))/(_sin(plane->elevation) * plane->speed) );
		_set_plane_x_y(plane, time_of_crash);
		_crashed_or_landed(plane);
		return 0;
	}
	else{
		plane->z = aux;
		_set_plane_x_y(plane, time_dif);
		return 1;
	}
}

static void _set_plane_x_y(struct atc_plane *plane, int time_dif)
{
	int aux = (int)(_cos(plane->heading) * plane->speed * time_dif + plane->x ) % MAX_LEN;
	if( aux < 0 ){
		aux = MAX_LEN - aux;
	}
	plane->x = aux;
	aux = (int)(_sin(plane->heading) * plane->speed * time_dif + plane->y ) % MAX_HEIGHT;
	if( aux < 0 ){
		aux = MAX_HEIGHT - aux;
	}
	plane->y = aux;

}

//Checkes if the plane was in a landing zone & satisfied landing conditions when it was at z = 0
//Landing conditions ar esped <= 200km/h & inclination -10
static void _crashed_or_landed(struct atc_plane *plane)
{
	if(plane->elevation != -10 || plane->speed > 56 || !_in_da_zone(plane) ){
		plane->status = crashed;
		_planes_count -= 1;
	}
	else{
		plane->status = landed;
	}
}

//Return 0 if the plane is IN DA ZONE!!
//Meaning if it was within LANDING_TOLERANCE of an airport when it reached z=0
static int _in_da_zone(struct atc_plane *plane)
{
	struct atc_airport airports[MAX_AIRPORTS];
	int a_count = get_airports(airports);
	for(int i =0; i < a_count; i++){
		if(plane->x > (airports[i].x - LANDING_TOLERANCE) && plane->x < (airports[i].x + LANDING_TOLERANCE) && 
			plane->y > (airports[i].y - LANDING_TOLERANCE) && plane->y < (airports[i].y + LANDING_TOLERANCE)){
			return 0;
		} 
	}
	return -1;
}

//hardcoded sin(x) values to avoid unnecesary calculation
static double _sin(int angle)
{
	switch(angle){
		case 0 : return 1.0;
		case 45 : return 0.7071;
		case 90 : return 0.0;
		case 135 : return -0.7071;
		case 180 : return -1.0;
		case 225 : return -0.7071;
		case 270 : return 0.0;
		case 315 : return 0.7071;
		default : return 0;
	}
}

//hardcoded cos(x) values to avoid unnecesary calculation
static float _cos(int angle)
{
	switch(angle){
		case -30 : return -0.5;
		case -20 : return -0.3420;
		case -10 : return -0.1736;
		case 0 : return 0.0;
		case 10 : return 0.1736;
		case 20 : return 0.3420;
		case 30 : return 0.5;
		case 45 : return 0.7071;
		case 90 : return 1.0;
		case 135 : return 0.7071;
		case 180 : return 0.0;
		case 225 : return -0.7071;
		case 270 : return -1;
		case 315 : return -0.7071;
		default : return 0;
	}
}


int get_airports(struct atc_airport buff[])
{
	struct atc_airport aux = {10, 10};
	struct atc_airport aux2 = {20, 20};
	buff[0] = aux;
	buff[1] = aux2;
	return 2;
}


//Creates a random plane, does not check if it already exist, because if it does.....it happens
//Also, chance of having 2 planes with an equal name is: 0.00000284478 which means that one out of every 351520 new planes will 
//have an already exisitng name plane, therefore we will not consider this case as to avoid acceses to the database.
void create_plane()
{
	if(_planes_count == MAX_PLANES){
		return;
	}
	srand(time(NULL));
	struct atc_plane new_plane;
	char id[6];
	id[0] = _rand_capital_letter();
	id[1] = _rand_capital_letter();
	id[2] = _rand_number();
	id[3] = _rand_number();
	id[4] = _rand_number();
	id[5] = _rand_capital_letter();

	memcpy(new_plane.id, id, 6);
	new_plane.x = rand()%MAX_LEN;
	new_plane.y = rand()%MAX_HEIGHT; 
	new_plane.z	= rand()%9000 + 1000; //starting altitude, expressed in m
	new_plane.time = time(NULL);
	new_plane.heading = (enum atc_heading)((rand() % 7) * 45);	
	new_plane.elevation	= (enum atc_elevation)( (rand()%7-6) *10);
	new_plane.speed	= rand()%195 + 55;
	new_plane.status = flying;

	sto_set(&sto_db, &new_plane, new_plane.id);
}

//auxiliar function
static int _rand_capital_letter()
{
	return (rand() % 26) +65;
}


static int _rand_number()
{
	return (rand() % 10) +48;
}


time_t get_time()
{
	return time(NULL);
}


//Return 0 if command was applied/succesfull, -1 if it was not a valid commando or not possible to apply
int set(enum atc_commands cmd, struct atc_plane plane)
{
	switch (cmd){
		case speed_up : if(plane.speed * 10 >= 280) { //maximum plane speed 1000km/h expressed in 28m/s
							return -1;
						}
						else{
							plane.speed += 14;
							return sto_set(&sto_db, &plane, plane.id);
						}
					break;
		case speed_down : if( plane.speed <= 42) {  //minimum plane speed 150km/h expressed in m/s
							return -1;
						}
						else{
							plane.speed -= 14;
							return sto_set(&sto_db, &plane, plane.id);
						}
					break;
		case climb : if( plane.elevation >= 30) { 
							return -1;
						}
						else{
							plane.elevation = plane.elevation + 10;
							return sto_set(&sto_db, &plane, plane.id);
						}
					break;
		case descend : if( plane.elevation <= -30) { 
							return -1;
						}
						else{
						 	plane.elevation = plane.elevation - 10;
							return sto_set(&sto_db, &plane, plane.id);
						}
					break;
		case turn_right : 	plane.heading = (plane.heading -45);
							return sto_set(&sto_db, &plane, plane.id);
					break;
		case turn_left: 	plane.heading = (plane.heading +45);
							return sto_set(&sto_db, &plane, plane.id);
					break;
		default: return 0;
	}
	return 0;
}


//Returns the amount of planes flying currently, which are loaded into the received plane buffer. Also, updates the storage
//when it detects that a plane has colided or landed
int get_airplanes(struct atc_plane buffer[])
{
	time_t current_time = get_time();
	struct atc_plane plane;
	int count = 0;
	struct sto_cursor query;
	sto_query(&query, &sto_db, (sto_filter)_flying_planes);

	int new_plane_flag = 1;
	new_plane_flag = sto_get(&query, &plane, NULL);

	while( (new_plane_flag != 0 && count < MAX_PLANES) ){
		if(calculate_position(&plane, current_time) ){
			memcpy(&buffer[count], &plane, sizeof(struct atc_plane));
			count++;
		}
		new_plane_flag = sto_get(&query, &plane, NULL);
	}

	_planes_count = count;	
	sto_close(&query);
	return count;
}


//Auxiliary function to start a query
static int _flying_planes(struct atc_plane plane, char name[6])
{
	return plane.status == flying;
}


//Initializes stuff

int atc_init()
{
	return sto_init(&sto_db, "Planes", sizeof(struct atc_plane));

}
