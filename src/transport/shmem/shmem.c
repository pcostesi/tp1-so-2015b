#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>

#include <assert.h>

#include "transport.h"

extern int errno;

/* Dirty little secret - We're cheating here by using a msg-sized buffer */
#define ZONE_SIZE (32)
#define SHM_SIZE (ZONE_SIZE * 2)
#define SRV_BASE (0)
#define CLI_BASE (ZONE_SIZE)
#define SHM_LISTEN_SIZE (sizeof(int))
#define SHM_LISTEN_ADDR "/SHM_LISTEN"
#define SHM_LISTEN_AVAILABLE "/SHM_LISTEN_SRV"
#define SHM_LISTEN_FREE "/SHM_LISTEN_CLI"
#define SHM_CONNECT_ADDR "/SHM_SOCK_%d"
#define SHM_CONNECT_AVAILABLE_SRV "/SHM_SOCK_%d_available_srv"
#define SHM_CONNECT_FREE_SRV "/SHM_SOCK_%d_free_srv"
#define SHM_CONNECT_AVAILABLE_CLI "/SHM_SOCK_%d_available_cli"
#define SHM_CONNECT_FREE_CLI "/SHM_SOCK_%d_free_cli"
#define OOPSIE fprintf(stderr, "\nInternal error %s at %s, line %d.\n", strerror(errno), __FILE__, __LINE__)
#define GUARD(A)	do { if ((A) == -1) { OOPSIE; return -1; } } while(0)
#define GUARD_SEM(A)	do { if ((A) == SEM_FAILED) { OOPSIE; return -1; } } while(0)

static int _shm_open_listen(struct transport_addr * addr, int flag);
static int _shm_close_listen(struct transport_addr * addr);
static int _shm_connect(struct transport_addr * addr);
static int _shm_close(struct transport_addr * addr);
static int _shm_setup_locks(struct transport_addr * addr);
static int _shm_unlocks(struct transport_addr * addr);

static int _shm_open_listen(struct transport_addr * addr, int flag)
{
    int shm_fd;
    void * ptr;

    shm_fd = shm_open(SHM_LISTEN_ADDR, O_RDWR | flag, 0666);
    GUARD(shm_fd);
    ftruncate(shm_fd, SHM_LISTEN_SIZE);

    ptr = mmap(0, SHM_LISTEN_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) {
        close(shm_fd);
        return -1;
    }

    addr->conn.shmem.fd = shm_fd;
    addr->conn.shmem.zone = ptr;
    return shm_fd;
}

static int _shm_close_listen(struct transport_addr * addr)
{
    GUARD(close(addr->conn.shmem.fd));
    GUARD(munmap(addr->conn.shmem.zone, SHM_LISTEN_SIZE));
    addr->conn.shmem.zone = NULL;
    addr->conn.shmem.connected = 0;
    addr->conn.shmem.fd = 0;
    return 0;
}


static int _shm_connect(struct transport_addr * addr)
{
    int shm_fd;
    void * ptr;
    char addrname[256] = {0};

    snprintf(addrname, sizeof(addrname), SHM_CONNECT_ADDR, addr->conn.shmem.port);
    shm_fd = shm_open(addrname, O_RDWR | O_CREAT, 0666);
    GUARD(shm_fd);
    ftruncate(shm_fd, SHM_SIZE);

    ptr = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) {
        OOPSIE;
        shm_unlink(addrname);
        close(shm_fd);
        return -1;
    }

    addr->conn.shmem.fd = shm_fd;
    addr->conn.shmem.zone = ptr;
    GUARD(_shm_setup_locks(addr));
    addr->conn.shmem.connected = 1;
    addr->conn.shmem.rd = 0;
    addr->conn.shmem.wr = 0;
    return shm_fd;
}

static int _shm_close(struct transport_addr * addr)
{
    char addrname[256] = {0};

    snprintf(addrname, sizeof(addrname), SHM_CONNECT_ADDR, addr->conn.shmem.port);
    close(addr->conn.shmem.fd);
    munmap(addr->conn.shmem.zone, SHM_SIZE);
    _shm_unlocks(addr);
    shm_unlink(SHM_CONNECT_ADDR);
    addr->conn.shmem.zone = NULL;
    addr->conn.shmem.connected = 0;
    return 0;
}

static int _shm_setup_locks(struct transport_addr * addr)
{
    char lock[256];

    snprintf(lock, sizeof(lock), SHM_CONNECT_AVAILABLE_SRV, addr->conn.shmem.port);
    addr->conn.shmem.locks.connection.available_srv = sem_open(lock, O_RDWR | O_CREAT, 0666, 0);
    GUARD_SEM(addr->conn.shmem.locks.connection.available_srv);

    snprintf(lock, sizeof(lock), SHM_CONNECT_FREE_SRV, addr->conn.shmem.port);
    addr->conn.shmem.locks.connection.free_srv = sem_open(lock, O_RDWR | O_CREAT, 0666, ZONE_SIZE);
    GUARD_SEM(addr->conn.shmem.locks.connection.free_srv);

    snprintf(lock, sizeof(lock), SHM_CONNECT_AVAILABLE_CLI, addr->conn.shmem.port);
    addr->conn.shmem.locks.connection.available_cli = sem_open(lock, O_RDWR | O_CREAT, 0666, 0);
    GUARD_SEM(addr->conn.shmem.locks.connection.available_cli);

    snprintf(lock, sizeof(lock), SHM_CONNECT_FREE_CLI, addr->conn.shmem.port);
    addr->conn.shmem.locks.connection.free_cli = sem_open(lock, O_RDWR | O_CREAT, 0666, ZONE_SIZE);
    GUARD_SEM(addr->conn.shmem.locks.connection.free_cli);

    return 0;
}

static int _shm_unlocks(struct transport_addr * addr)
{
    char lock[256];

    snprintf(lock, sizeof(lock), SHM_CONNECT_AVAILABLE_SRV, addr->conn.shmem.port);
    (sem_close(addr->conn.shmem.locks.connection.available_srv));
    addr->conn.shmem.locks.connection.available_srv = NULL;

    snprintf(lock, sizeof(lock), SHM_CONNECT_FREE_SRV, addr->conn.shmem.port);
    (sem_close(addr->conn.shmem.locks.connection.free_srv));
    addr->conn.shmem.locks.connection.free_srv = NULL;

    snprintf(lock, sizeof(lock), SHM_CONNECT_AVAILABLE_CLI, addr->conn.shmem.port);
    (sem_close(addr->conn.shmem.locks.connection.available_cli));
    addr->conn.shmem.locks.connection.available_cli = NULL;

    snprintf(lock, sizeof(lock), SHM_CONNECT_FREE_CLI, addr->conn.shmem.port);
    (sem_close(addr->conn.shmem.locks.connection.free_cli));
    addr->conn.shmem.locks.connection.free_cli = NULL;

    return 0;
}

int transport_send(struct transport_addr * addr, unsigned char * buffer, size_t size)
{
    int bytes = 0;

    char * base_offset;
    sem_t * available;
    sem_t * free;

    base_offset = addr->conn.shmem.zone;
    if (addr->conn.shmem.connected == 0) return -1;
    if (addr->conn.shmem.i_am_the_server) {
        base_offset += SRV_BASE;
        available = addr->conn.shmem.locks.connection.available_srv;
        free = addr->conn.shmem.locks.connection.free_srv;
    } else {
        base_offset += CLI_BASE;
        available = addr->conn.shmem.locks.connection.available_cli;
        free = addr->conn.shmem.locks.connection.free_cli;
    }

    while (bytes < size) {
        GUARD(sem_wait(free));
        base_offset[addr->conn.shmem.wr] = buffer[bytes];
        addr->conn.shmem.wr = (addr->conn.shmem.wr + 1) % ZONE_SIZE;
        bytes += 1;
        GUARD(sem_post(available));
    }

    return bytes;
}

int transport_recv(struct transport_addr * addr, unsigned char * buffer, size_t size)
{
    int bytes = 0;

    char * base_offset;
    sem_t * available;
    sem_t * free;

    base_offset = addr->conn.shmem.zone;
    if (addr->conn.shmem.connected == 0) return -1;
    if (addr->conn.shmem.i_am_the_server) {
        base_offset += CLI_BASE;
        available = addr->conn.shmem.locks.connection.available_cli;
        free = addr->conn.shmem.locks.connection.free_cli;
    } else {
        base_offset += SRV_BASE;
        available = addr->conn.shmem.locks.connection.available_srv;
        free = addr->conn.shmem.locks.connection.free_srv;
    }

    while (bytes < size) {
        GUARD(sem_wait(available));
        buffer[bytes] = base_offset[addr->conn.shmem.rd];
        addr->conn.shmem.rd = (addr->conn.shmem.rd + 1) % ZONE_SIZE;
        bytes += 1;
        GUARD(sem_post(free));
    }

    return bytes;
}


int transport_connect(struct transport_addr * addr)
{
    sem_t * available;
    sem_t * free;
    
    addr->conn.shmem.i_am_the_server = 0;
    GUARD_SEM(available = sem_open(SHM_LISTEN_AVAILABLE, O_RDWR));
    GUARD_SEM(free = sem_open(SHM_LISTEN_FREE, O_RDWR));

    GUARD(_shm_open_listen(addr, 0));
    if (sem_trywait(available) == -1) {
        _shm_close_listen(addr);
        return -1;
    }
    memcpy(&addr->conn.shmem.port, addr->conn.shmem.zone, SHM_LISTEN_SIZE);
    GUARD(_shm_close_listen(addr));
    sem_post(free);

    GUARD(sem_close(available));
    GUARD(sem_close(free));

    GUARD(_shm_connect(addr));
    return 0;
}

int transport_listen(struct transport_addr * addr)
{
    addr->conn.shmem.port = 0;
    addr->conn.shmem.i_am_the_server = 1;
    addr->conn.shmem.i_am_the_listen = 1;

    shm_unlink(SHM_LISTEN_ADDR);

    sem_unlink(SHM_LISTEN_AVAILABLE);
    addr->conn.shmem.locks.listen.available = sem_open(SHM_LISTEN_AVAILABLE, O_RDWR | O_CREAT, 0666, 0);
    GUARD_SEM(addr->conn.shmem.locks.listen.available);

    sem_unlink(SHM_LISTEN_FREE);
    addr->conn.shmem.locks.listen.free = sem_open(SHM_LISTEN_FREE, O_RDWR | O_CREAT, 0666, 0);
    GUARD_SEM(addr->conn.shmem.locks.listen.free);

    GUARD(_shm_open_listen(addr, O_CREAT));
    addr->conn.shmem.connected = 1;

    return 0;
}

int transport_close(struct transport_addr * addr)
{
    if (addr->conn.shmem.connected == 0) return 0;
    addr->conn.shmem.connected = 0;

    if (addr->conn.shmem.i_am_the_listen) {
        _shm_close_listen(addr);
        sem_close(addr->conn.shmem.locks.listen.available);
        sem_close(addr->conn.shmem.locks.listen.free);
        return 0;
    }
    GUARD(_shm_close(addr));
    return 0;
}

int transport_serv_init(struct transport_addr * addr)
{
    addr->conn.shmem.i_am_the_server = 1;
    addr->conn.shmem.port = 0;
    addr->conn.shmem.connected = 0;
    return 0;
}

int transport_accept(struct transport_addr * addr, struct transport_addr * worker)
{
    worker->conn.shmem.i_am_the_server = 1;
    memcpy(addr->conn.shmem.zone, &addr->conn.shmem.port, SHM_LISTEN_SIZE);
    worker->conn.shmem.port = addr->conn.shmem.port;
    addr->conn.shmem.port += 1;
    GUARD(_shm_connect(worker));
    sem_post(addr->conn.shmem.locks.listen.available);

    sem_wait(addr->conn.shmem.locks.listen.free);
    return 1;
}

