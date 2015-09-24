#include "cli.h"
#include "atcd.h"
#include "protocol.h"




int start_cli()
{
	return atc_connect();
}

int close_cli()
{
	return atc_close();
}


int get_planes(struct atc_plane buff[])
{

}


int get_airports(void)
{

}


void create_plane(void)
{

}


int set(enum atc_command)
{

}



int atcd_test(void) {
    return -1;
}

/*atc connect  listen   close   -   protocol.h*/
/**/
