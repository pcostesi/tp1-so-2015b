#include "atcd.h"
#include <stdio.h>

int main(int argc, char ** arcv)
{
    int atcd_res = atcd_test();
    printf("This is the server calling itself: %d\n", atcd_res);

    return 0;
}
