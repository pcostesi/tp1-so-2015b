#include <stdio.h>
#include "atcd.h"
#include "atcc.h"
#include "ui.h"

int main(int argc, char ** argv)
{
    printf("hello! %d\n", atcd_test());
    ui_test();
    return 0;
}
