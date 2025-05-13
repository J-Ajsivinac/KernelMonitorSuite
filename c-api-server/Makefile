CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -Iexternal
SRC = \
    src/main.c \
    src/json_utils.c \
    src/routes.c \
    src/system_process.c \
    src/system_kill.c \
    src/energy.c \
    src/network.c \
    src/resources.c \
    external/mongoose.c \
    external/cJSON.c
OUT = server

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(OUT)
