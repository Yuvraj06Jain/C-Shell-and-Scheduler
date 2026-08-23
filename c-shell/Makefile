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

STANDALONE = reveal peek locate

ALL_SRCS   = $(wildcard $(SRC_DIR)/*.c)
SHELL_SRCS = $(filter-out $(addprefix $(SRC_DIR)/,$(addsuffix .c,$(STANDALONE))), $(ALL_SRCS))
SHELL_OBJS = $(SHELL_SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

all: $(BIN) reveal peek locate

$(BIN): $(SHELL_OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

reveal: $(OBJ_DIR)/reveal.o
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

peek: $(OBJ_DIR)/peek.o
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^


locate: $(OBJ_DIR)/locate.o $(OBJ_DIR)/pathUtils.o
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

clean:
	rm -rf $(OBJ_DIR) $(BIN) reveal peek locate

.PHONY: all clean