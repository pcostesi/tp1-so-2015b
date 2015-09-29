#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>

#include "transport.h"

/* Dirty little secret - We're cheating here by using a msg-sized buffer */
#define ATC_SHM_SIZE (1024 * 2)
#define MAX_SEM_LEN 256
#define ATC_SHM_SRV "/ATC_SHM_SRV"
#define ATC_SHM_CLI "/atc_conn_%d"
#define ATC_DATA_SRV "/atc_conn_%d_data_srv"
#define ATC_BUSY_SRV "/atc_conn_%d_busy_srv"
#define ATC_DATA_CLI "/atc_conn_%d_data_cli"
#define ATC_BUSY_CLI "/atc_conn_%d_busy_cli"
#define GUARD(A)	do { if ((A) == -1) puts("OH FUCK"); return -1; } while(0)
#define GUARD_SEM(A)	do { if ((A) == SEM_FAILED) return -1; } while(0)

static int _shm_connect(struct transport_addr * addr);
static int _shm_close(struct transport_addr * addr);
static int _shm_locks(struct transport_addr * addr);
static int _shm_unlocks(struct transport_addr * addr);
static int _shm_update_addr(struct transport_addr * addr);
static int _shm_lock_for_reading(struct transport_addr * addr);
static int _shm_lock_for_writing(struct transport_addr * addr);
static int _shm_unlock_after_reading(struct transport_addr * addr);
static int _shm_unlock_after_writing(struct transport_addr * addr);

static int _shm_connect(struct transport_addr * addr)
{
	int shm_fd;
	void * ptr;

	shm_fd = shm_open(addr->conn.shmem.name, O_RDWR | O_CREAT, 0666);
	GUARD(shm_fd);
	ftruncate(shm_fd, ATC_SHM_SIZE);

	ptr = mmap(0, ATC_SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	if (ptr == MAP_FAILED) {
		close(shm_fd);
		return -1;
	}

	addr->conn.shmem.fd = shm_fd;
	addr->conn.shmem.zone = ptr;
	GUARD(_shm_locks(addr));
	addr->conn.shmem.connected = 1;
	return shm_fd;
}

static int _shm_close(struct transport_addr * addr)
{
	if (!addr->conn.shmem.connected) return -1;
	GUARD(close(addr->conn.shmem.fd));
	GUARD(shm_unlink(addr->conn.shmem.name));
	GUARD(munmap(addr->conn.shmem.zone, ATC_SHM_SIZE));
	GUARD(_shm_unlocks(addr));
	addr->conn.shmem.zone = NULL;
	addr->conn.shmem.connected = 0;
	return 0;
}

static int _shm_update_addr(struct transport_addr * addr)
{
	int buffer = getpid();
	puts("before send");
	if (transport_send(addr, (void *) &buffer, sizeof(buffer)) == -1) {
		puts("oh crap");
		_shm_close(addr);
		return -1;
	}
	puts("after sending");
	_shm_close(addr);
	GUARD(snprintf(addr->conn.shmem.name, sizeof(addr->conn.shmem.name), ATC_SHM_CLI, buffer));
	addr->conn.shmem.no = buffer;
	return _shm_connect(addr);
}

static int _shm_locks(struct transport_addr * addr)
{
	char buffer[MAX_SEM_LEN];
	snprintf(buffer, sizeof(buffer), ATC_DATA_SRV, addr->conn.shmem.no);
	if (addr->conn.shmem.i_am_the_server)
		GUARD_SEM((addr->conn.shmem.data_srv = sem_open(buffer, O_RDWR|O_CREAT, 0666, 0)));
	else
		GUARD_SEM((addr->conn.shmem.data_srv = sem_open(buffer, O_RDWR)));

	snprintf(buffer, sizeof(buffer), ATC_BUSY_SRV, addr->conn.shmem.no);
	if (addr->conn.shmem.i_am_the_server)
		GUARD_SEM((addr->conn.shmem.busy_srv = sem_open(buffer, O_RDWR|O_CREAT, 0666, 1)));
	else
		GUARD_SEM((addr->conn.shmem.busy_srv = sem_open(buffer, O_RDWR)));

	snprintf(buffer, sizeof(buffer), ATC_DATA_CLI, addr->conn.shmem.no);
	if (addr->conn.shmem.i_am_the_server)
		GUARD_SEM((addr->conn.shmem.data_cli = sem_open(buffer, O_RDWR|O_CREAT, 0666, 0)));
	else
		GUARD_SEM((addr->conn.shmem.data_cli = sem_open(buffer, O_RDWR)));

	snprintf(buffer, sizeof(buffer), ATC_BUSY_CLI, addr->conn.shmem.no);
	if (addr->conn.shmem.i_am_the_server)
		GUARD_SEM((addr->conn.shmem.busy_cli = sem_open(buffer, O_RDWR|O_CREAT, 0666, 1)));
	else
		GUARD_SEM((addr->conn.shmem.busy_cli = sem_open(buffer, O_RDWR)));

	return 0;
}

static int _shm_unlocks(struct transport_addr * addr)
{
	char buffer[MAX_SEM_LEN];
	snprintf(buffer, sizeof(buffer), ATC_DATA_SRV, addr->conn.shmem.no);
	GUARD(sem_unlink(buffer));
	GUARD(sem_close(addr->conn.shmem.data_srv));

	snprintf(buffer, sizeof(buffer), ATC_BUSY_SRV, addr->conn.shmem.no);
	GUARD(sem_unlink(buffer));
	GUARD(sem_close(addr->conn.shmem.busy_srv));

	snprintf(buffer, sizeof(buffer), ATC_DATA_CLI, addr->conn.shmem.no);
	GUARD(sem_unlink(buffer));
	GUARD(sem_close(addr->conn.shmem.data_cli));

	snprintf(buffer, sizeof(buffer), ATC_BUSY_CLI, addr->conn.shmem.no);
	GUARD(sem_unlink(buffer));
	GUARD(sem_close(addr->conn.shmem.busy_cli));

	puts("unlocking locks");
	return 0;
}

static int _shm_lock_for_reading(struct transport_addr * addr)
{
	puts("locking for reading");
	if (addr->conn.shmem.i_am_the_server) {
		puts("im the server, bitch");
		sem_wait(addr->conn.shmem.data_srv);
		sem_wait(addr->conn.shmem.busy_srv);
	} else {
		sem_wait(addr->conn.shmem.data_cli);
		sem_wait(addr->conn.shmem.busy_cli);
	}
	return 0;
}

static int _shm_lock_for_writing(struct transport_addr * addr)
{
	puts("locking for writing");
	if (addr->conn.shmem.i_am_the_server) {
		puts("im the server, bitch");
		sem_wait(addr->conn.shmem.busy_cli);
	} else {
		sem_wait(addr->conn.shmem.busy_srv);
	}
	return 0;
}

static int _shm_unlock_after_reading(struct transport_addr * addr)
{
	puts("unlocking after reading");
	if (addr->conn.shmem.i_am_the_server) {
		puts("im the server, bitch");
		sem_post(addr->conn.shmem.busy_srv);
	} else {
		sem_post(addr->conn.shmem.busy_cli);
	}
	return 0;
}

static int _shm_unlock_after_writing(struct transport_addr * addr)
{
	if (addr->conn.shmem.i_am_the_server) {
		puts("im the server, bitch");
		sem_post(addr->conn.shmem.busy_cli);
		sem_post(addr->conn.shmem.data_cli);
	} else {
		sem_post(addr->conn.shmem.busy_srv);
		sem_post(addr->conn.shmem.data_srv);
	}
	return 0;
}

int transport_send(struct transport_addr * addr, unsigned char * buffer, size_t size)
{
	int bytes = -1;

	GUARD(_shm_lock_for_writing(addr));
	puts("sending");
	size_t base_offset = addr->conn.shmem.i_am_the_server ? ATC_SHM_SIZE / 2 : 0;
	memcpy(addr->conn.shmem.zone + base_offset, buffer, size);
	bytes = size;
	GUARD(_shm_unlock_after_writing(addr));

	return bytes;
}

int transport_recv(struct transport_addr * addr, unsigned char * buffer, size_t size)
{
	int bytes = -1;

	GUARD(_shm_lock_for_reading(addr));
	puts("receiving");
	size_t base_offset = addr->conn.shmem.i_am_the_server ? 0 : ATC_SHM_SIZE / 2;
	memcpy(buffer, addr->conn.shmem.zone + base_offset, size);
	bytes = size;
	GUARD(_shm_unlock_after_reading(addr));

	return bytes;
}


int transport_connect(struct transport_addr * addr)
{
	addr->conn.shmem.i_am_the_server = 0;
	addr->conn.shmem.no = 0;
	/* out of time to actually initialize the addr, we hard-coded it. */
	strncpy(addr->conn.shmem.name, ATC_SHM_SRV, sizeof(addr->conn.shmem.name));
	puts("connecting to:");
	puts(addr->conn.shmem.name);
	GUARD(_shm_connect(addr));
	puts("connected!");
	/* read new address */
	GUARD(_shm_update_addr(addr));
	puts("upgraded");
	return 0;
}

int transport_listen(struct transport_addr * addr)
{
	addr->conn.shmem.no = 0;
	addr->conn.shmem.i_am_the_server = 1;
	GUARD(_shm_connect(addr));
	return 0;
}

int transport_close(struct transport_addr * addr)
{
	GUARD(_shm_close(addr));
	return _shm_unlocks(addr);
}

int transport_serv_init(struct transport_addr * addr)
{
	addr->conn.shmem.i_am_the_server = 1;
	/* out of time to actually initialize the addr, we hard-coded it. */
	strncpy(addr->conn.shmem.name, ATC_SHM_SRV, sizeof(addr->conn.shmem.name));
	puts("base addr");
	puts(addr->conn.shmem.name);
	return 0;
}

int transport_accept(struct transport_addr * addr, struct transport_addr * worker)
{
	int buffer;
	puts("before reading buffer");
	GUARD(transport_recv(addr, (void *) &buffer, sizeof(buffer)));
	worker->conn.shmem.i_am_the_server = 1;
	puts("buffer ok");

	GUARD(snprintf(worker->conn.shmem.name, sizeof(worker->conn.shmem.name), ATC_SHM_CLI, buffer));
	puts(worker->conn.shmem.name);
	GUARD(_shm_connect(worker));
	puts("worker connected");
	addr->conn.shmem.no = buffer;
	return 0;
}

