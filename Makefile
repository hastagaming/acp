CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c11
TARGET = acp
SRC_DIR = src
SOURCES = $(SRC_DIR)/main.c $(SRC_DIR)/git.c $(SRC_DIR)/parser.c $(SRC_DIR)/remote.c $(SRC_DIR)/safety.c $(SRC_DIR)/ignore.c $(SRC_DIR)/error.c $(SRC_DIR)/util.c $(SRC_DIR)/config.c

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET)

.PHONY: all clean