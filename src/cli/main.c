#include "atcd.h"

int calculate_position(struct atc_plane * plane, time_t time)
{
    return -1;
}

int atcd_test(void)
{
    return 42;
}

void atc_new_game(void)
{
}

/* you can't return this! Please check it!!! */
struct atc_plane new_plane(void)
{
    struct atc_plane the_plane;
    return the_plane;
}

time_t get_time(void)
{
    return -1;
}

void blip(void)
{
}

int get_airports(struct atc_airport ports[])
{
    return -1;
}

int set(enum atc_commands cmd, struct atc_plane plane)
{
    return -1;
}

int get_airplanes(struct atc_plane buffer[])
{
    return -1;
}

int atc_init(void)
{
    return 0;
}

void create_plane(void)
{
}

