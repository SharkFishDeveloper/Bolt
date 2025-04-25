# Compiler
CC = gcc

# Directories
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
BIN_DIR = bin

# Files
SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
EXEC = index

# Flags

CFLAGS = -I$(INC_DIR)

# Targets
all: mkdirs $(EXEC)

$(EXEC): $(OBJ) 
	$(CC) -o $@ $^ 

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	rm -rf $(OBJ_DIR)/*.o $(EXEC)

run: all
	$(EXEC)

mkdirs:
	@if not exist $(OBJ_DIR) mkdir $(OBJ_DIR)
	@if not exist $(BIN_DIR) mkdir $(BIN_DIR)

.PHONY: all clean mkdirs run
