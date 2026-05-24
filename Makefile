CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic -Isrc/common -Isrc/client -Isrc/server
LDFLAGS_CLIENT = -lncurses -lsodium
LDFLAGS_SERVER = -lsodium

SRC_COMMON = src/common/crypto.c src/common/utils.c src/common/buffer.c
SRC_CLIENT = src/client/main.c src/client/ui.c src/client/network.c src/client/commands.c
SRC_SERVER = src/server/main.c src/server/server.c src/server/clients.c src/server/rooms.c src/server/auth.c

OBJ_COMMON = $(SRC_COMMON:src/%.c=build/obj/%.o)
OBJ_CLIENT = $(SRC_CLIENT:src/%.c=build/obj/%.o)
OBJ_SERVER = $(SRC_SERVER:src/%.c=build/obj/%.o)

all: dirs build/bin/client build/bin/server

dirs:
	mkdir -p build/bin build/obj/common build/obj/client build/obj/server

build/bin/client: $(OBJ_COMMON) $(OBJ_CLIENT)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS_CLIENT)

build/bin/server: $(OBJ_COMMON) $(OBJ_SERVER)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS_SERVER)

build/obj/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf build/
