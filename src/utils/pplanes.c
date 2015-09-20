#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "storage.h"
#include "atcd.h"

void print_row(struct atc_plane * plane, char key[STO_KEY_SIZE])
{
    printf("%*s:\t %6s\t <%3d, %3d, %3d>\t %d, %d, %d, %c, %d\n",
            STO_KEY_SIZE,
            key,
            plane->id,
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
    char key[STO_KEY_SIZE];

    if (argc != 2) {
        printf("Usage: %s database.\n", argv[0]);
        return -1;
    }

    assert(sto_init(&db, argv[1], sizeof(struct atc_plane)) != -1);


    printf("Database %s:\n", argv[1]);
    assert(sto_query(&q, &db, NULL) != -1);
    
    while ((sto_get(&q, &plane, key) != -1) && !sto_key_empty(key)) {
        print_row(&plane, key);
    }
    
    assert(sto_close(&q) != -1);

    printf("Done.\n");
    return 0;
}
