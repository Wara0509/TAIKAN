CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -pedantic -Iinclude
TARGET = taikan
TUI_TARGET = taikan_tui

SRCS = src/main.c src/logic.c src/ui_cli.c src/ui_tui_stub.c
TUI_SRCS = src/main.c src/logic.c src/ui_cli.c src/gui_tui.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

$(TUI_TARGET): $(TUI_SRCS)
	$(CC) $(CFLAGS) -o $(TUI_TARGET) $(TUI_SRCS) -lncurses

tui: $(TUI_TARGET)

run: $(TARGET)
	./$(TARGET)

run-tui: $(TUI_TARGET)
	./$(TUI_TARGET)

clean:
	rm -f $(TARGET) $(TUI_TARGET)
