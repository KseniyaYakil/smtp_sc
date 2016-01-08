CC = gcc
CFLAGS = -Wall -Werror -std=c99 -ggdb3

INC = include
SRC_DIR = src
OBJ_DIR = obj
SRC = server.c
OBJ = $(SRC:.c=.o)
LIB = -lconfig
TARGET = smtp_server

SRC_ALL = $(patsubst %.c,$(SRC_DIR)/%.c, $(SRC))
OBJ_ALL = $(patsubst %.o,$(OBJ_DIR)/%.o, $(OBJ))

all: server

server: obj_create server_build

obj_create:
	mkdir -p $(OBJ_DIR)
	$(CC) -c -o $(OBJ_ALL) $(SRC_ALL) $(CFLAGS) -I $(INC) $(LIB)

server_build:
	$(CC) -o $(TARGET) $(OBJ_ALL) $(LIB)

clean:
	rm -f $(OBJ_ALL) $(TARGET)

.PHONY: server clean
