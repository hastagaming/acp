CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c11
TARGET = acp
SRC_DIR = src
SOURCES = $(SRC_DIR)/main.c $(SRC_DIR)/git.c $(SRC_DIR)/parser.c $(SRC_DIR)/remote.c $(SRC_DIR)/safety.c $(SRC_DIR)/ignore.c $(SRC_DIR)/error.c $(SRC_DIR)/util.c $(SRC_DIR)/config.c $(SRC_DIR)/tag.c

# On Termux, $PREFIX is already set by the environment (e.g.
# /data/data/com.termux/files/usr) and is used as-is. On a regular Linux
# desktop, $PREFIX is normally unset, so it falls back to /usr/local,
# the standard location for software installed outside the system
# package manager.
PREFIX ?= /usr/local
BIN_DIR = $(PREFIX)/bin
CONFIG_DIR = $(PREFIX)/etc/acp

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

install: $(TARGET)
	mkdir -p $(BIN_DIR)
	cp $(TARGET) $(BIN_DIR)/$(TARGET)
	chmod 755 $(BIN_DIR)/$(TARGET)
	mkdir -p $(CONFIG_DIR)
	cp config/default.conf $(CONFIG_DIR)/default.conf
	@echo ""
	@echo "Installed: $(BIN_DIR)/$(TARGET)"
	@echo "Config:    $(CONFIG_DIR)/default.conf"
	@echo "Try running: acp --help"

uninstall:
	rm -f $(BIN_DIR)/$(TARGET)
	rm -rf $(CONFIG_DIR)
	@echo "Removed $(BIN_DIR)/$(TARGET) and $(CONFIG_DIR)"

clean:
	rm -f $(TARGET)

.PHONY: all install uninstall clean