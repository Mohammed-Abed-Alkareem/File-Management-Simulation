# Variables
CC = gcc
CFLAGS = -Wall -Wextra -O2 -Iinclude
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Programs
MAIN_EXE = $(BIN_DIR)/file-management-simulation
SUB_PROGRAMS = $(BIN_DIR)/calculator $(BIN_DIR)/generator \
               $(BIN_DIR)/inspector1 $(BIN_DIR)/inspector2 \
               $(BIN_DIR)/inspector3 $(BIN_DIR)/mover

# Libraries
LIBS = -lGL -lGLU -lglut -lpthread -lm

# Dependencies

CONFIG_OBJ = $(OBJ_DIR)/config.o
UTILS_OBJ = $(OBJ_DIR)/utils.o

# All executables
all: $(MAIN_EXE) $(SUB_PROGRAMS)

# Main program
$(MAIN_EXE): $(OBJ_DIR)/main.o $(CONFIG_OBJ) $(UTILS_OBJ)
	@mkdir -p $(BIN_DIR)
	$(CC) $^ -o $@ $(LIBS)

# Subprograms
$(BIN_DIR)/calculator: $(OBJ_DIR)/calculator.o $(CONFIG_OBJ) $(UTILS_OBJ)
	@mkdir -p $(BIN_DIR)
	$(CC) $^ -o $@ $(LIBS)

$(BIN_DIR)/generator: $(OBJ_DIR)/generator.o $(CONFIG_OBJ) $(UTILS_OBJ)
	@mkdir -p $(BIN_DIR)
	$(CC) $^ -o $@ $(LIBS)

$(BIN_DIR)/inspector1: $(OBJ_DIR)/inspector1.o $(CONFIG_OBJ) $(UTILS_OBJ)
	@mkdir -p $(BIN_DIR)
	$(CC) $^ -o $@ $(LIBS)

$(BIN_DIR)/inspector2: $(OBJ_DIR)/inspector2.o $(CONFIG_OBJ) $(UTILS_OBJ)
	@mkdir -p $(BIN_DIR)
	$(CC) $^ -o $@ $(LIBS)

$(BIN_DIR)/inspector3: $(OBJ_DIR)/inspector3.o $(CONFIG_OBJ) $(UTILS_OBJ)
	@mkdir -p $(BIN_DIR)
	$(CC) $^ -o $@ $(LIBS)

$(BIN_DIR)/mover: $(OBJ_DIR)/mover.o $(CONFIG_OBJ) $(UTILS_OBJ)
	@mkdir -p $(BIN_DIR)
	$(CC) $^ -o $@ $(LIBS)

# Compile common object file
$(CONFIG_OBJ): $(SRC_DIR)/config.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(UTILS_OBJ): $(SRC_DIR)/utils.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@



# Compile other object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean
