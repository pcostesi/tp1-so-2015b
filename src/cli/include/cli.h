#ifndef __ATCD_H
#define __ATCD_H 1

#include "protocol.h"



/*Function Headers*/

int start_cli();

int close_cli();

int get_airports(void);

void create_plane(void);

int set(enum atc_command);

#endif