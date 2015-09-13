#include "storage.h"
#include <fcntl.h>



int sto_open(struct sto_database * conn, char * name)
{
    return 0;
}

int sto_close(struct sto_database * conn)
{
    return 0;
}

int sto_get(struct sto_query * q, void * row, char * key[STO_KEY_SIZE])
{
    return 0;
}

int sto_set(struct sto_database * conn, void * row, char key[STO_KEY_SIZE])
{
    return 0;
}

int sto_dispose(struct sto_database * conn)
{
    return 0;
}

int sto_when(struct sto_query * q, sto_fn when, void * buffer, void * extra)
{
    return 0;
}

