#include "atcd.h"

int atcd_test(void)
{
    return 5;
}


//TODO int GET_AIRPLANES(BUFFER)

static void _set_plane_x_y(struct atc_plane *plane, int time_dif);
static void _crashed_or_landed(struct atc_plane *plane);
static int _in_da_zone(struct atc_plane *plane);
static int _sin(int angle);
static float _cos(int angle);


void calculate_position(struct atc_plane* plane, time_t new_time)
{
	int time_dif = new_time - plane->time;
	int aux = (int)(_sin(plane->elevation) * plane->speed * time_dif + plane->z ) ;
	if( aux < 0 ){
		int time_of_crash = new_time - plane->time - ( (aux*(-1))/(_sin(plane->elevation) * plane->speed) );
		_set_plane_x_y(plane, time_of_crash);
		_crashed_or_landed(plane);
	}
	else{
		plane->z = aux;
		_set_plane_x_y(plane, time_dif);
	}

	//TODO CHECK TIME FORMAT, NEED INT

	
}

static void _set_plane_x_y(struct atc_plane *plane, int time_dif)
{
	int aux = (int)(_cos(plane->heading) * plane->speed * time_dif + plane->x ) % MAX_LEN;
	if( aux < 0 ){
		aux= MAX_LEN - aux;
	}
	plane->x = aux;
	aux = (int)(_sin(plane->heading) * plane->speed * time_dif + plane->y ) % MAX_HEIGHT;
	if( aux < 0 ){
		aux= MAX_HEIGHT - aux;
	}
	plane->y= aux;

}

static void _crashed_or_landed(struct atc_plane *plane)
{
	if(plane->elevation != -10 || plane->speed >200 || !_in_da_zone(plane) ){
		plane->status = crashed;
	}
	else{
		plane->status = landed;
	}
}

//Return 1 if the plane is IN DA ZONE!!
static int _in_da_zone(struct atc_plane *plane)
{
	struct atc_airport airports[MAX_AIRPORTS];
	int a_count = get_airports(airports);
	for(int i =0; i < a_count; i++){
		if(plane->x > (airports[i].x - LANDING_TOLERANCE) && plane->x < (airports[i].x + LANDING_TOLERANCE) && 
			plane->y > (airports[i].y - LANDING_TOLERANCE) && plane->y < (airports[i].y + LANDING_TOLERANCE)){
			return 1;
		} 
	}
	return 0;
}

static int _sin(int angle)
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
	return 0;
}
