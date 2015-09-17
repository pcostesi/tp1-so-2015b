#ifndef __STO_H
#define __STO_H 1

#include <fcntl.h>
#include <stddef.h>
#include <string.h>

#define STO_MAX_FILENAME_SIZE 128
#define STO_MAX_NAME_LENGTH 64
#define STO_KEY_SIZE 16

typedef int (* sto_filter)(void * data, char key[STO_KEY_SIZE]);
typedef int (* sto_fn)(void * data, void * extra, char key[STO_KEY_SIZE]);

struct sto_database {
    size_t row_size;
    size_t data_size;
    char name[STO_MAX_NAME_LENGTH + 1];
    char path[STO_MAX_FILENAME_SIZE + 1];
};

struct sto_cursor {
    int fd;
    struct flock zone_lock;
    unsigned int cursor;
    struct sto_database * database;
    sto_filter filter;
};


int sto_init(struct sto_database * conn, char * name, size_t data_type_size);
int sto_close(struct sto_cursor * q);
int sto_query(struct sto_cursor * q, struct sto_database * conn, sto_filter filter);
int sto_get(struct sto_cursor * q, void * row, char key[STO_KEY_SIZE]);
int sto_set(struct sto_database * conn, void * row, const char key[STO_KEY_SIZE]);
int sto_dispose(struct sto_database * conn);
int sto_when(struct sto_cursor * q, sto_fn when, void * buffer, void * extra);
int sto_del(struct sto_database * conn, void * row, char key[STO_KEY_SIZE]);
int sto_key_empty(char key[STO_KEY_SIZE]);
#endif
