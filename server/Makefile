CC = gcc
CFLAGS = -Wall -Werror -std=c99 -ggdb3

INC = include
SRC_DIR = src
OBJ_DIR = obj
LIB = -lconfig
TARGET = smtp_server

SRC = server.c conn.c session.c
OBJECTS = $(patsubst %.o,$(OBJ_DIR)/%.o, $(SRC:.c=.o))

all: server

server: $(OBJECTS)
	$(CC) -o $(TARGET) $(OBJECTS) $(LIB)

$(OBJECTS): $(OBJ_DIR)/%.o : $(SRC_DIR)/%.c
	mkdir -p $(OBJ_DIR)
	$(CC) -c $< $(CFLAGS) -I $(INC) $(LIB) -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: server clean
