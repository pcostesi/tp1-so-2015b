#include "storage.h"

#include <stddef.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define ROUND_UP(number, mask) (((number) + (mask) - 2) & ~((mask) - 1))

/* Internal prototypes */

static int _acquire_global_write_lock(struct flock * lock, int fd);
static int _acquire_lock(struct flock * lock, int fd, int l_type, int pos, int size);
static int _acquire_r_lock(struct flock * lock, int fd, int pos, int size);
static int _acquire_w_lock(struct flock * lock, int fd, int pos, int size);
static int _release_lock(struct flock * lock, int fd);
static int _fetch_next_record(struct sto_cursor * q, void * buffer, char key[STO_KEY_SIZE]);
static int _noop_cmp(void * v, char key[STO_KEY_SIZE]);

/* Public API */

int sto_init(struct sto_database * conn, char * name, size_t data_type_size)
{
    if (conn == NULL || data_type_size == 0 || name == NULL) {
        return -1;
    }
    strncpy(conn->path, name, STO_MAX_FILENAME_SIZE);
    strncpy(conn->name, name, STO_MAX_NAME_LENGTH);
    conn->data_size = data_type_size;
    /* store things 32-bit aligned. */
    conn->row_size = ROUND_UP(STO_KEY_SIZE + data_type_size, 32);
    return 0;
}

int sto_query(struct sto_cursor * q, struct sto_database * conn, sto_filter filter)
{
    if (q == NULL || conn == NULL) {
        return -1;
    }

    if (filter == NULL) {
        filter = (sto_filter) _noop_cmp;
    }

    q->fd = open(conn->path, O_CREAT | O_RDWR | O_DSYNC, S_IRUSR | S_IWUSR);
    if (q->fd == -1) {
        return -1;
    }

    q->database = conn;
    q->filter = filter;
    q->cursor = 0;
    lseek(q->fd, 0, SEEK_SET);
    return 0;
}

int sto_close(struct sto_cursor * q)
{
    if (q == NULL) {
        return -1;
    }
    return close(q->fd);
}

int sto_get(struct sto_cursor * q, void * row, char key[STO_KEY_SIZE])
{
    int res = 0;

    _acquire_r_lock(&q->zone_lock, q->fd, 0, 0);
    while ((res = _fetch_next_record(q, row, key)) != 0) {
        if (res == -1) {
            return -1;
        }
        if (q->filter(row, key)) {
            return res;
        }
    }
    _release_lock(&q->zone_lock, q->fd);
    return 0;
}

int sto_get_by_id(struct sto_database * db, void * row, char key[STO_KEY_SIZE])
{
    struct sto_cursor q;
    char tmp_key[STO_KEY_SIZE];
    int res = 0;
    strncpy(tmp_key, key, STO_KEY_SIZE);

    sto_query(db, &q, (sto_filter) _sto_key_equals, key);
    res = sto_get(&q, row, tmp_key);
    if (res == -1) {
        sto_close(&q);
        return -1;
    }
    sto_close(&q);
    return res;
}

int sto_key_empty(char key[STO_KEY_SIZE])
{
    char empty[STO_KEY_SIZE] = {0};
    return !memcmp(key, empty, STO_KEY_SIZE);
}

int sto_set(struct sto_database * conn, void * row, const char key[STO_KEY_SIZE])
{
    struct sto_cursor q;
    char tmp_key[STO_KEY_SIZE];
    int res;
    int key_len = 0;
    int bytes_written = 0;

    if (sto_query(&q, conn, NULL, NULL) == -1) {
        return -1;
    }

    key_len = strnlen(key, STO_KEY_SIZE);

    _acquire_w_lock(&q.zone_lock, q.fd, 0, 0);
    while ((res = _fetch_next_record(&q, NULL, tmp_key)) != 0) {
        if (res == -1) {
            sto_close(&q);
            _release_lock(&q.zone_lock, q.fd);
            return -1;
        }
        if (strncmp(tmp_key, key, key_len) == 0) {
            break;
        }
    }
   
    if (sto_key_empty(tmp_key)) {
        memset(tmp_key, 0, STO_KEY_SIZE);
        strncpy(tmp_key, key, key_len);
        bytes_written = write(q.fd, tmp_key, STO_KEY_SIZE);
        if (bytes_written == -1) {
            sto_close(&q);
            _release_lock(&q.zone_lock, q.fd);
            return -1;
        }
    }

    bytes_written = write(q.fd, row, conn->data_size);
    if (bytes_written == -1) {
        sto_close(&q);
        _release_lock(&q.zone_lock, q.fd);
        return -1;
    }
    
    sto_close(&q);
    _release_lock(&q.zone_lock, q.fd);
    return bytes_written;
}

int sto_del(struct sto_database * conn, void * row, char key[STO_KEY_SIZE])
{
    return 0;
}

int sto_dispose(struct sto_database * conn)
{
    struct flock clean;
    int fd;

    fd = open(conn->path, O_CREAT | O_WRONLY | O_DSYNC, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        return -1;
    }
    
    if (_acquire_global_write_lock(&clean, fd) == -1) {
        close(fd);
        return -1;
    }
    if (ftruncate(fd, 0) == -1) {
        close(fd);
        return -1;
    }

    if (_release_lock(&clean, fd) == -1) {
        close(fd);
        return -1;
    }
    return 0;
}

int sto_when(struct sto_cursor * q, sto_fn when, void * buffer, void * extra)
{
    return 0;
}

/* Helper functions and other dragons */

static int _noop_cmp(void * v, char key[STO_KEY_SIZE], void * unused)
{
    return 1;
}

static int _sto_key_equals(void * v, char key[STO_KEY_SIZE], void * tmp_key)
{
    return strncmp(key, ) != 0;
}

static int _fetch_next_record(struct sto_cursor * q, void * buffer,
                              char key[STO_KEY_SIZE])
{
    size_t offset;
    int bytes_read;
    struct sto_database * db;
    char tmp_key[STO_KEY_SIZE];
    char * _key = key != NULL ? key : tmp_key;

    db = q->database;
    offset = q->cursor * db->row_size;
   
    bytes_read = lseek(q->fd, offset, SEEK_SET);
    if (bytes_read == -1) {
        return -1;
    }
    bytes_read = read(q->fd, _key, STO_KEY_SIZE);
    if (bytes_read == 0) {
        memset(_key, 0, STO_KEY_SIZE);
        if (buffer != NULL) memset(buffer, 0, db->data_size);
        return 0;
    }

    if (buffer != NULL) {
        bytes_read = read(q->fd, buffer, db->data_size);
    }

    q->cursor += 1;

    return bytes_read;
}

static int _acquire_r_lock(struct flock * lock, int fd, int pos, int size)
{
    return _acquire_lock(lock, fd, F_RDLCK, pos, size);
}

static int _acquire_w_lock(struct flock * lock, int fd, int pos, int size)
{
    return _acquire_lock(lock, fd, F_WRLCK, pos, size);
}

static int _acquire_lock(struct flock * lock, int fd, int l_type, int pos, int size)
{
    lock->l_type = l_type;
    lock->l_whence = SEEK_SET;
    lock->l_start = pos;
    lock->l_len = size;
    lock->l_pid = getpid();
    return fcntl(fd, F_SETLKW, lock);
}

static int _acquire_global_write_lock(struct flock * lock, int fd)
{
    return _acquire_lock(lock, fd, F_WRLCK, 0, 0);
}

static int _release_lock(struct flock * lock, int fd)
{
    lock->l_type = F_UNLCK;
    return fcntl(fd, F_SETLKW, lock);
}

#undef ROUND_UP

