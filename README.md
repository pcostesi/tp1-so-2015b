# Air Traffic Controller



## Installation

Requirements:
- Make
- A modern C compiler (Clang, GCC, std99)
- A UNIX environment
- ncurses

The project has multiple binaries. The default target `all` builds `atc`
(a stand-alone executable), `atcd-socket` and `atcc-socket`. The transport
can be changed overriding the `BACKEND` parameter and defaults to `socket`.

Examples:

    make atc

Builds the stand-alone binary

    make BACKEND="pipe"

Builds `atc`, `atcd-pipe`, and `atcc-pipe`.

### Notes on NCurses

For Ubuntu 14.04

    apt-get install libncurses5-dev

## Debugging

The binaries are compiled with `-O0 -g` by default. GDB and Valgrind can
be used for debugging.

## Structure

The project is divided into six main modules and some extra support files:
- **ATCC**: Frontend, client side logic.
- **CLI**: Re-implements the _ATCD_ functions as proxy calls to the API.
- **COMM**: A bi-directional communication, backend-agnostic layer.
  - Pipe
  - File
  - Socket
  - Queue
  - Shmem
- **SRV**: Server side logic, does all the forking and client management, while
  child processes call the backend functions.
- **ATCD**: Game logic, keeps the game state and runs the simulation.
- **STO**: Data storage abstraction layer (w/locking + concurrency control.)
- **msg.h**: Common message/protocol header.


Combo table:

| ATCC | CLI | COMM - msg.h | SRV | ATCD - STO |     Result     |
|:----:|:---:|:------------:|:---:|:----------:|:--------------:|
|  X   |     |              |     |     X      |       atc      |
|  X   |  X  |    socket    |     |            |  atcc-socket   |
|      |     |    socket    |  X  |     X      |  atcd-socket   |
|  X   |  X  |    shmem     |     |            |  atcc-shmem    |
|      |     |    shmem     |  X  |     X      |  atcd-shemm    |



## Compilation Details

Make will re-compile the binaries when the follwing files get updated:

|     Condition     | atc | atcc-${BACKEND} | atcd-${BACKEND} |
|:-----------------:|----:|----------------:|----------------:|
| Any \*.h          |  X  |       X         |        X        |
| atcc/\*.c         |  X  |       X         |                 |
| atcd/\*.c         |  X  |                 |        X        |
| transport/\*.c    |     |       X         |        X        |
| transport/\*\*.c  |     |       X         |        X        |
| protocol/\*.c     |     |       X         |        X        |
| cli/\*.c          |     |       X         |                 |
| srv/\*.c          |     |                 |        X        |
| storage/\*.c      |  X  |                 |        X        |

Additionally, _Make_ will set these preprocessor variables: `ATC_BACKEND_$(BACKEND_IN_UPPERCASE)` and `ATC_BACKEND=$(BACKEND_IN_UPPERCASE)`.
