CC       = gcc
CFLAGS   = -std=c23 \
           -D_POSIX_C_SOURCE=200809L \
           -D_XOPEN_SOURCE=700 \
           -Wall -Wextra -Werror \
           -Wno-unused-parameter \
           -fno-asm 

INCLUDES = -Iinclude

SRC_DIR  = src
OBJ_DIR  = obj
BIN      = shell

SRCS     = $(wildcard $(SRC_DIR)/*.c)
OBJS     = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR) $(BIN)

.PHONY: all clean