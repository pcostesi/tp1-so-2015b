#include <stdio.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include "storage.h"
#include "atcd.h"

void print_row(struct atc_plane * plane, char key[STO_KEY_SIZE])
{
    printf("Key %*s: <%4d, %4d, %4d>\t t%d, h%3d, e%3d, s%3d, x%d\n",
            STO_KEY_SIZE,
            key,
            plane->x, plane->y, plane->z,
            (int) plane->time,
            plane->heading,
            plane->elevation,
            plane->speed,
            plane->status);
}


int main(int argc, char ** argv)
{
    struct sto_database db;
    struct sto_cursor q;
    struct atc_plane plane;
    time_t start_time;
    time_t last_time;

    char key[STO_KEY_SIZE];

    if (argc != 2) {
        printf("Usage: %s database.\n", argv[0]);
        return -1;
    }

    start_time = time(NULL);

    assert(sto_init(&db, argv[1], sizeof(struct atc_plane)) != -1);


    printf("Database %s:\n", argv[1]);
    assert(sto_query(&q, &db, NULL) != -1);
    
    while ((sto_get(&q, &plane, key) != -1) && !sto_key_empty(key)) {
        print_row(&plane, key);
    }
    
    assert(sto_close(&q) != -1);

    last_time = time(NULL);
    printf("Done in %ld seconds.\n", last_time - start_time);
    return 0;
}
