CC = gcc
CFLAGS = -Wall -Iinclude


ifdef __DEBUG
	CFLAGS += -D__DEBUG
endif

ifdef __CLI
    CFLAGS += -D__CLI
else
$(info )
$(info There will be no print statements to the CLI)
$(info If you want to see print statements, please run 'make __CLI=1')
$(info )
endif

ifdef __UI
    CFLAGS += -D__UI
else
$(info )
$(info There will be no UI *_*)
$(info If you want to see the UI, please run 'make __UI=1')
$(info )
endif

# Directories
SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin

#
#
##
#
#
#
# 


# Create the obj directory if it doesn't exist
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Create the bin directory if it doesn't exist
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Clean up build files
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# Phony targets
.PHONY: all clean
