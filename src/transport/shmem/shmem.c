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
#define ATC_SHM_SRV "/ATC_SHM_SRV"
#define ATC_SHM_CLI "/atc_conn_%d"
#define ATC_SEND_SRV "/atc_conn_%d_send_srv"
#define ATC_RECV_SRV "/atc_conn_%d_recv_srv"
#define ATC_SEND_CLI "/atc_conn_%d_send_cli"
#define ATC_RECV_CLI "/atc_conn_%d_recv_cli"

static int _shm_connect(struct transport_addr * addr);
static int _shm_close(struct transport_addr * addr);
static int _shm_locks(struct transport_addr * addr);
static int _shm_unlocks(struct transport_addr * addr);
static int _shm_update_addr(struct transport_addr * addr);

static int _shm_connect(struct transport_addr * addr)
{
	int shm_fd;
	void * ptr;

	shm_fd = shm_open(addr->conn.shmem.name, O_RDWR | O_CREAT, 0666);
	if (shm_fd == -1) {
		return -1;
	}
	ftruncate(shm_fd, ATC_SHM_SIZE);

	ptr = mmap(0, ATC_SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	if (ptr == MAP_FAILED) {
		close(shm_fd);
		return -1;
	}

	addr->conn.shmem.fd = shm_fd;
	addr->conn.shmem.zone = ptr;
	addr->conn.shmem.no = 0;
	if (_shm_locks(addr) == -1) return -1;
	return shm_fd;
}

static int _shm_close(struct transport_addr * addr)
{
	if (close(addr->conn.shmem.fd) == -1) return -1;
	if (shm_unlink(addr->conn.shmem.name) == -1) return -1;
	if (munmap(addr->conn.shmem.zone, ATC_SHM_SIZE) == -1) return -1;
	free(addr->conn.shmem.zone);
	addr->conn.shmem.zone = NULL;
	return 0;
}

static int _shm_update_addr(struct transport_addr * addr)
{
	int buffer = getpid();
	if (transport_send(addr, (void *) &buffer, sizeof(buffer)) == -1) {
		_shm_close(addr);
		return -1;
	}
	//_shm_close(addr);
	if (snprintf(addr->conn.shmem.name, sizeof(addr->conn.shmem.name), "/atc_conn_%d", buffer) == -1)
		return -1;
	addr->conn.shmem.no = buffer;
	return _shm_connect(addr);
}

static int _shm_locks(struct transport_addr * addr)
{
	char buffer[256];
	snprintf(buffer, sizeof(buffer), ATC_SEND_SRV, addr->conn.shmem.no);
	if ((addr->conn.shmem.send_srv = sem_open(buffer, O_RDWR|O_CREAT, 0666, 1)) == NULL) return -1;

	snprintf(buffer, sizeof(buffer), ATC_SEND_CLI, addr->conn.shmem.no);
	if ((addr->conn.shmem.send_cli = sem_open(buffer, O_RDWR|O_CREAT, 0666, 1)) == NULL) return -1;

	snprintf(buffer, sizeof(buffer), ATC_RECV_SRV, addr->conn.shmem.no);
	if ((addr->conn.shmem.recv_srv = sem_open(buffer, O_RDWR|O_CREAT, 0666, 0)) == NULL) return -1;

	snprintf(buffer, sizeof(buffer), ATC_RECV_CLI, addr->conn.shmem.no);
	if ((addr->conn.shmem.recv_cli = sem_open(buffer, O_RDWR|O_CREAT, 0666, 0)) == NULL) return -1;

	return 0;
}

static int _shm_unlocks(struct transport_addr * addr)
{
	return 0; //-1;
}

int transport_send(struct transport_addr * addr, unsigned char * buffer, size_t size)
{
	int bytes = -1;

	if (addr->conn.shmem.i_am_the_server) {
		sem_wait(addr->conn.shmem.send_srv);
	} else {
		sem_wait(addr->conn.shmem.send_cli);
	}

	size_t base_offset = addr->conn.shmem.i_am_the_server ? ATC_SHM_SIZE / 2 : 0;
	memcpy(addr->conn.shmem.zone + base_offset, buffer, size);

	bytes = size;

	if (addr->conn.shmem.i_am_the_server) {
		sem_post(addr->conn.shmem.recv_cli);
	} else {
		sem_post(addr->conn.shmem.recv_srv);
	}

	return bytes;
}

int transport_recv(struct transport_addr * addr, unsigned char * buffer, size_t size)
{
	int bytes = -1;

	if (addr->conn.shmem.i_am_the_server) {
		sem_wait(addr->conn.shmem.recv_srv);
	} else {
		sem_wait(addr->conn.shmem.recv_cli);
	}

	size_t base_offset = addr->conn.shmem.i_am_the_server ? 0 : ATC_SHM_SIZE / 2;
	memcpy(buffer, addr->conn.shmem.zone + base_offset, size);

	bytes = size;

	if (addr->conn.shmem.i_am_the_server) {
		sem_post(addr->conn.shmem.send_cli);
	} else {
		sem_post(addr->conn.shmem.send_srv);
	}
	return bytes;
}


int transport_connect(struct transport_addr * addr)
{
	addr->conn.shmem.i_am_the_server = 0;
	/* out of time to actually initialize the addr, we hard-coded it. */
	strncpy(addr->conn.shmem.name, ATC_SHM_SRV, sizeof(addr->conn.shmem.name));
	if (_shm_connect(addr) == -1) return -1;
	/* read new address */
	if (_shm_update_addr(addr) == -1) return -1;
	return 0;
}

int transport_listen(struct transport_addr * addr)
{
	addr->conn.shmem.i_am_the_server = 1;
	if (_shm_connect(addr) == -1) return -1;
	return 0;
}

int transport_close(struct transport_addr * addr)
{
	if (_shm_close(addr) == -1) return -1;
	return _shm_unlocks(addr);
}

int transport_serv_init(struct transport_addr * addr)
{
	/* out of time to actually initialize the addr, we hard-coded it. */
	strncpy(addr->conn.shmem.name, ATC_SHM_SRV, sizeof(addr->conn.shmem.name));
	return 0;
}

int transport_accept(struct transport_addr * addr, struct transport_addr * worker)
{
	int buffer;
	if (transport_recv(addr, (void *) &buffer, sizeof(buffer)) == -1) return -1;
	worker->conn.shmem.i_am_the_server = 1;

	if (snprintf(worker->conn.shmem.name, sizeof(worker->conn.shmem.name), ATC_SHM_CLI, buffer) == -1)
		return -1;
	if (_shm_connect(worker) == -1) return -1;

	addr->conn.shmem.no = buffer;
	return 0;
}

