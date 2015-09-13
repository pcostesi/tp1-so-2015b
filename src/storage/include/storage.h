#ifndef __STO_H
#define __STO_H 1

#include <fcntl.h>

#define STO_MAX_NAME_LENGTH 64
#define STO_KEY_SIZE 16

typedef int (* sto_filter)(void * data, char key[STO_KEY_SIZE]);
typedef int (* sto_fn)(void * data, void * extra, char key[STO_KEY_SIZE]);

struct sto_database {
    int fd;
    size_t row_size;
    char name[STO_MAX_NAME_LENGTH];
};

struct sto_query {
    struct flock reading;
    size_t cursor;
    struct sto_database * database;
    sto_filter filter;
};


int sto_open(struct sto_database * conn, char * name);
int sto_close(struct sto_database * conn);
int sto_get(struct sto_query * q, void * row, char * key[STO_KEY_SIZE]);
int sto_set(struct sto_database * conn, void * row, char key[STO_KEY_SIZE]);
int sto_dispose(struct sto_database * conn);
int sto_when(struct sto_query * q, sto_fn when, void * buffer, void * extra);

/*
int sto_del(struct sto_database * conn, void * row, char key[STO_KEY_SIZE]);
 */
#endif
